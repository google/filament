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

#include <string>
#include <tuple>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/ir/ternary.h"
#include "src/tint/lang/hlsl/type/int8_t4_packed.h"
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
        // Find the bitcasts that need replacing.
        Vector<core::ir::Bitcast*, 4> bitcast_worklist;
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* bitcast = inst->As<core::ir::Bitcast>()) {
                bitcast_worklist.Push(bitcast);
                continue;
            }
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kAcosh:
                    case core::BuiltinFn::kAsinh:
                    case core::BuiltinFn::kAtanh:
                    case core::BuiltinFn::kAtomicAdd:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicMin:
                    case core::BuiltinFn::kAtomicMax:
                    case core::BuiltinFn::kAtomicAnd:
                    case core::BuiltinFn::kAtomicOr:
                    case core::BuiltinFn::kAtomicXor:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kAtomicStore:
                    case core::BuiltinFn::kAtomicExchange:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kCountOneBits:
                    case core::BuiltinFn::kDot4I8Packed:
                    case core::BuiltinFn::kDot4U8Packed:
                    case core::BuiltinFn::kFrexp:
                    case core::BuiltinFn::kModf:
                    case core::BuiltinFn::kPack2X16Float:
                    case core::BuiltinFn::kPack2X16Snorm:
                    case core::BuiltinFn::kPack2X16Unorm:
                    case core::BuiltinFn::kPack4X8Snorm:
                    case core::BuiltinFn::kPack4X8Unorm:
                    case core::BuiltinFn::kPack4XI8:
                    case core::BuiltinFn::kPack4XU8:
                    case core::BuiltinFn::kPack4XI8Clamp:
                    case core::BuiltinFn::kQuantizeToF16:
                    case core::BuiltinFn::kReverseBits:
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kSign:
                    case core::BuiltinFn::kSubgroupAnd:
                    case core::BuiltinFn::kSubgroupOr:
                    case core::BuiltinFn::kSubgroupXor:
                    case core::BuiltinFn::kSubgroupShuffleXor:
                    case core::BuiltinFn::kSubgroupShuffleUp:
                    case core::BuiltinFn::kSubgroupShuffleDown:
                    case core::BuiltinFn::kSubgroupInclusiveAdd:
                    case core::BuiltinFn::kSubgroupInclusiveMul:
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
                    case core::BuiltinFn::kTrunc:
                    case core::BuiltinFn::kUnpack2X16Float:
                    case core::BuiltinFn::kUnpack2X16Snorm:
                    case core::BuiltinFn::kUnpack2X16Unorm:
                    case core::BuiltinFn::kUnpack4X8Snorm:
                    case core::BuiltinFn::kUnpack4X8Unorm:
                    case core::BuiltinFn::kUnpack4XI8:
                    case core::BuiltinFn::kUnpack4XU8:
                        call_worklist.Push(call);
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the bitcasts that we found.
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
            } else {
                ReplaceBitcastWithAs(bitcast);
            }
        }

        // Replace the builtin calls that we found
        for (auto* call : call_worklist) {
            switch (call->Func()) {
                case core::BuiltinFn::kAcosh:
                    Acosh(call);
                    break;
                case core::BuiltinFn::kAsinh:
                    Asinh(call);
                    break;
                case core::BuiltinFn::kAtanh:
                    Atanh(call);
                    break;
                case core::BuiltinFn::kAtomicAdd:
                    AtomicAdd(call);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicSub(call);
                    break;
                case core::BuiltinFn::kAtomicMin:
                    AtomicMin(call);
                    break;
                case core::BuiltinFn::kAtomicMax:
                    AtomicMax(call);
                    break;
                case core::BuiltinFn::kAtomicAnd:
                    AtomicAnd(call);
                    break;
                case core::BuiltinFn::kAtomicOr:
                    AtomicOr(call);
                    break;
                case core::BuiltinFn::kAtomicXor:
                    AtomicXor(call);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicLoad(call);
                    break;
                case core::BuiltinFn::kAtomicStore:
                    AtomicStore(call);
                    break;
                case core::BuiltinFn::kAtomicExchange:
                    AtomicExchange(call);
                    break;
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(call);
                    break;
                case core::BuiltinFn::kCountOneBits:
                    BitcastToIntOverloadCall(call);  // See crbug.com/tint/1550.
                    break;
                case core::BuiltinFn::kDot4I8Packed:
                    Dot4I8Packed(call);
                    break;
                case core::BuiltinFn::kDot4U8Packed:
                    Dot4U8Packed(call);
                    break;
                case core::BuiltinFn::kFrexp:
                    Frexp(call);
                    break;
                case core::BuiltinFn::kModf:
                    Modf(call);
                    break;
                case core::BuiltinFn::kPack2X16Float:
                    Pack2x16Float(call);
                    break;
                case core::BuiltinFn::kPack2X16Snorm:
                    Pack2x16Snorm(call);
                    break;
                case core::BuiltinFn::kPack2X16Unorm:
                    Pack2x16Unorm(call);
                    break;
                case core::BuiltinFn::kPack4X8Snorm:
                    Pack4x8Snorm(call);
                    break;
                case core::BuiltinFn::kPack4X8Unorm:
                    Pack4x8Unorm(call);
                    break;
                case core::BuiltinFn::kPack4XI8:
                    Pack4xI8(call);
                    break;
                case core::BuiltinFn::kPack4XU8:
                    Pack4xU8(call);
                    break;
                case core::BuiltinFn::kPack4XI8Clamp:
                    Pack4xI8Clamp(call);
                    break;
                case core::BuiltinFn::kQuantizeToF16:
                    QuantizeToF16(call);
                    break;
                case core::BuiltinFn::kReverseBits:
                    BitcastToIntOverloadCall(call);  // See crbug.com/tint/1550.
                    break;
                case core::BuiltinFn::kSelect:
                    Select(call);
                    break;
                case core::BuiltinFn::kSign:
                    Sign(call);
                    break;
                case core::BuiltinFn::kSubgroupAnd:
                case core::BuiltinFn::kSubgroupOr:
                case core::BuiltinFn::kSubgroupXor:
                    BitcastToIntOverloadCall(call);
                    break;
                case core::BuiltinFn::kSubgroupShuffleXor:
                case core::BuiltinFn::kSubgroupShuffleUp:
                case core::BuiltinFn::kSubgroupShuffleDown:
                    SubgroupShuffle(call);
                    break;
                case core::BuiltinFn::kSubgroupInclusiveAdd:
                case core::BuiltinFn::kSubgroupInclusiveMul:
                    SubgroupInclusive(call);
                    break;
                case core::BuiltinFn::kTextureDimensions:
                    TextureDimensions(call);
                    break;
                case core::BuiltinFn::kTextureGather:
                    TextureGather(call);
                    break;
                case core::BuiltinFn::kTextureGatherCompare:
                    TextureGatherCompare(call);
                    break;
                case core::BuiltinFn::kTextureLoad:
                    TextureLoad(call);
                    break;
                case core::BuiltinFn::kTextureNumLayers:
                    TextureNumLayers(call);
                    break;
                case core::BuiltinFn::kTextureNumLevels:
                    TextureNumLevels(call);
                    break;
                case core::BuiltinFn::kTextureNumSamples:
                    TextureNumSamples(call);
                    break;
                case core::BuiltinFn::kTextureSample:
                    TextureSample(call);
                    break;
                case core::BuiltinFn::kTextureSampleBias:
                    TextureSampleBias(call);
                    break;
                case core::BuiltinFn::kTextureSampleCompare:
                case core::BuiltinFn::kTextureSampleCompareLevel:
                    TextureSampleCompare(call);
                    break;
                case core::BuiltinFn::kTextureSampleGrad:
                    TextureSampleGrad(call);
                    break;
                case core::BuiltinFn::kTextureSampleLevel:
                    TextureSampleLevel(call);
                    break;
                case core::BuiltinFn::kTextureStore:
                    TextureStore(call);
                    break;
                case core::BuiltinFn::kTrunc:
                    Trunc(call);
                    break;
                case core::BuiltinFn::kUnpack2X16Float:
                    Unpack2x16Float(call);
                    break;
                case core::BuiltinFn::kUnpack2X16Snorm:
                    Unpack2x16Snorm(call);
                    break;
                case core::BuiltinFn::kUnpack2X16Unorm:
                    Unpack2x16Unorm(call);
                    break;
                case core::BuiltinFn::kUnpack4X8Snorm:
                    Unpack4x8Snorm(call);
                    break;
                case core::BuiltinFn::kUnpack4X8Unorm:
                    Unpack4x8Unorm(call);
                    break;
                case core::BuiltinFn::kUnpack4XI8:
                    Unpack4xI8(call);
                    break;
                case core::BuiltinFn::kUnpack4XU8:
                    Unpack4xU8(call);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
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
            auto* mul = b.Multiply(result_ty, args[0], args[0]);
            auto* sub = b.Subtract(result_ty, mul, one);
            auto* sqrt = b.Call(result_ty, core::BuiltinFn::kSqrt, sub);
            auto* add = b.Add(result_ty, args[0], sqrt);
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
            auto* mul = b.Multiply(result_ty, args[0], args[0]);
            auto* add_one = b.Add(result_ty, mul, one);
            auto* sqrt = b.Call(result_ty, core::BuiltinFn::kSqrt, add_one);
            auto* add = b.Add(result_ty, args[0], sqrt);
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
            auto* one_plus_x = b.Add(result_ty, one, args[0]);
            auto* one_minus_x = b.Subtract(result_ty, one, args[0]);
            auto* div = b.Divide(result_ty, one_plus_x, one_minus_x);
            auto* log = b.Call(result_ty, core::BuiltinFn::kLog, div);
            auto* mul = b.Multiply(result_ty, log, half);

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
            auto* val = b.Subtract(type, b.Zero(type), args[1]);
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
            b.ConstructWithResult(call->DetachResult(), o, b.Equal(ty.bool_(), o, cmp));
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
        Vector<core::ir::Value*, 4> args = call->Args();
        auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(call->DetachResult(), args);
        ternary->InsertBefore(call);
        call->Destroy();
    }

    // HLSL's trunc is broken for very large/small float values.
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
            args.Push(b.LessThan(ty.MatchWidth(ty.bool_(), type), val, b.Zero(type))->Result());
        });
        auto* trunc = b.ir.CreateInstruction<hlsl::ir::Ternary>(call->DetachResult(), args);
        trunc->InsertBefore(call);

        call->Destroy();
    }

    /// Replaces an identity bitcast result with the value.
    void ReplaceBitcastWithValue(core::ir::Bitcast* bitcast) {
        bitcast->Result()->ReplaceAllUsesWith(bitcast->Val());
        bitcast->Destroy();
    }

    void ReplaceBitcastWithAs(core::ir::Bitcast* bitcast) {
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
        auto* src_type = bitcast->Val()->Type();
        if (src_type->IsIntegerVector()) {
            if (auto* c = bitcast->Val()->As<core::ir::Constant>()) {
                castToSrcType = c->Value()->Is<core::constant::Splat>();
            }
        }

        b.InsertBefore(bitcast, [&] {
            auto* source = bitcast->Val();
            if (castToSrcType) {
                source = b.Construct(src_type, source)->Result();
            }
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn, source);
        });
        bitcast->Destroy();
    }

    // Bitcast f16 types to others by converting the given f16 value to f32 and call
    // f32tof16 to get the bits. This should be safe, because the conversion is precise
    // for finite and infinite f16 value as they are exactly representable by f32.
    core::ir::Function* CreateBitcastFromF16(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(src_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in HLSL):
                //
                // uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
                //   uint2 r = f32tof16(float2(src));
                //   return uint((r.x & 65535u) | ((r.y & 65535u) << 16u));
                // }

                auto fn_name = b.ir.symbols.New(std::string("tint_bitcast_from_f16")).Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    auto* src_vec = src_type->As<core::type::Vector>();

                    auto* cast = b.Convert(ty.vec(ty.f32(), src_vec->Width()), src);
                    auto* r =
                        b.Let("r", b.Call<hlsl::ir::BuiltinCall>(ty.vec(ty.u32(), src_vec->Width()),
                                                                 hlsl::BuiltinFn::kF32Tof16, cast));

                    auto* x = b.And(ty.u32(), b.Swizzle(ty.u32(), r, {0_u}), 0xffff_u);
                    auto* y = b.ShiftLeft(
                        ty.u32(), b.And(ty.u32(), b.Swizzle(ty.u32(), r, {1_u}), 0xffff_u), 16_u);

                    auto* s = b.Or(ty.u32(), x, y);
                    core::ir::InstructionResult* result = nullptr;

                    switch (src_vec->Width()) {
                        case 2: {
                            result = s->Result();
                            break;
                        }
                        case 4: {
                            auto* z = b.And(ty.u32(), b.Swizzle(ty.u32(), r, {2_u}), 0xffff_u);
                            auto* w = b.ShiftLeft(
                                ty.u32(), b.And(ty.u32(), b.Swizzle(ty.u32(), r, {3_u}), 0xffff_u),
                                16_u);

                            auto* t = b.Or(ty.u32(), z, w);
                            auto* cons = b.Construct(ty.vec2<u32>(), s, t);
                            result = cons->Result();
                            break;
                        }
                        default:
                            TINT_UNREACHABLE();
                    }

                    tint::Switch(
                        dst_type->DeepestElement(),  //
                        [&](const core::type::F32*) {
                            b.Return(f, b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat,
                                                                      result));
                        },
                        [&](const core::type::I32*) {
                            b.Return(f, b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsint,
                                                                      result));
                        },
                        [&](const core::type::U32*) { b.Return(f, result); },  //
                        TINT_ICE_ON_NO_MATCH);
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the FromF16 polyfill for the given types
    void ReplaceBitcastWithFromF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastFromF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    // Bitcast other types to f16 types by reinterpreting their bits as f16 using
    // f16tof32, and convert the result f32 to f16. This should be safe, because the
    // conversion is precise for finite and infinite f16 result value as they are
    // exactly representable by f32.
    core::ir::Function* CreateBitcastToF16(const core::type::Type* src_type,
                                           const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(dst_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in HLSL):
                //
                // vector<float16_t, 2> tint_bitcast_to_f16(float src) {
                //   uint v = asuint(src);
                //   float t_low = f16tof32(v & 65535u);
                //   float t_high = f16tof32((v >> 16u) & 65535u);
                //   return vector<float16_t, 2>(t_low.x, t_high.x);
                // }

                auto fn_name = b.ir.symbols.New(std::string("tint_bitcast_to_f16")).Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    const core::type::Type* uint_ty = ty.MatchWidth(ty.u32(), src_type);
                    const core::type::Type* float_ty = ty.MatchWidth(ty.f32(), src_type);

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

                    core::ir::Value* mask = nullptr;
                    core::ir::Value* shift = nullptr;
                    if (src_vec) {
                        mask = b.Let("mask", b.Splat(uint_ty, 0xffff_u))->Result();
                        shift = b.Let("shift", b.Splat(uint_ty, 16_u))->Result();
                    } else {
                        mask = b.Value(b.Constant(0xffff_u));
                        shift = b.Value(b.Constant(16_u));
                    }

                    auto* l = b.And(uint_ty, v, mask);
                    auto* t_low = b.Let(
                        "t_low", b.Call<hlsl::ir::BuiltinCall>(float_ty, BuiltinFn::kF16Tof32, l));

                    auto* h = b.And(uint_ty, b.ShiftRight(uint_ty, v, shift), mask);
                    auto* t_high = b.Let(
                        "t_high", b.Call<hlsl::ir::BuiltinCall>(float_ty, BuiltinFn::kF16Tof32, h));

                    core::ir::Instruction* x = nullptr;
                    core::ir::Instruction* y = nullptr;
                    if (src_vec) {
                        x = b.Swizzle(ty.f32(), t_low, {0_u});
                        y = b.Swizzle(ty.f32(), t_high, {0_u});
                    } else {
                        x = t_low;
                        y = t_high;
                    }
                    x = b.Convert(ty.f16(), x);
                    y = b.Convert(ty.f16(), y);

                    auto dst_width = dst_type->As<core::type::Vector>()->Width();
                    TINT_ASSERT(dst_width == 2 || dst_width == 4);

                    if (dst_width == 2) {
                        b.Return(f, b.Construct(dst_type, x, y));
                    } else {
                        auto* z = b.Convert(ty.f16(), b.Swizzle(ty.f32(), t_low, {1_u}));
                        auto* w = b.Convert(ty.f16(), b.Swizzle(ty.f32(), t_high, {1_u}));
                        b.Return(f, b.Construct(dst_type, x, y, z, w));
                    }
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

    /// Replaces a bitcast with a call to the ToF16 polyfill for the given types
    void ReplaceBitcastWithToF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastToF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    void TextureNumLayers(core::ir::CoreBuiltinCall* call) {
        auto* tex = call->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        TINT_ASSERT(tex_type->Dim() == core::type::TextureDimension::k2dArray ||
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
                TINT_ICE() << "texture dimension is kNone";
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
        bool has_level = call->Args().Length() > 1;
        bool is_ms =
            tex_type
                ->IsAnyOf<core::type::MultisampledTexture, core::type::DepthMultisampledTexture>();

        Vector<uint32_t, 2> swizzle{};
        uint32_t query_size = 0;
        switch (tex_type->Dim()) {
            case core::type::TextureDimension::kNone:
                TINT_ICE() << "texture dimension is kNone";
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

        TINT_ASSERT(tex_type->Dim() == core::type::TextureDimension::k2d);
        TINT_ASSERT((tex_type->IsAnyOf<core::type::DepthMultisampledTexture,
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
                    call_args.Push(b.Construct(ty.vec2<i32>(), coord, lvl)->Result());
                    break;
                }
                case core::type::TextureDimension::k2d: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec2<i32>(), args[1]);
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
                        call_args.Push(b.Construct(ty.vec3<i32>(), coord, lvl)->Result());
                    }
                    break;
                }
                case core::type::TextureDimension::k2dArray: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec2<i32>(), args[1]);
                    auto* ary_idx = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                    core::ir::Value* lvl = nullptr;
                    if (is_storage) {
                        lvl = b.Constant(0_i);
                    } else {
                        lvl = b.InsertConvertIfNeeded(ty.i32(), args[3]);
                    }
                    call_args.Push(b.Construct(ty.vec4<i32>(), coord, ary_idx, lvl)->Result());
                    break;
                }
                case core::type::TextureDimension::k3d: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec3<i32>(), args[1]);
                    core::ir::Value* lvl = nullptr;
                    if (is_storage) {
                        lvl = b.Constant(0_i);
                    } else {
                        lvl = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                    }
                    call_args.Push(b.Construct(ty.vec4<i32>(), coord, lvl)->Result());
                    break;
                }
                default:
                    TINT_UNREACHABLE();
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
        TINT_ASSERT(tex_type);

        Vector<core::ir::Value*, 3> new_args;
        new_args.Push(tex);

        b.InsertBefore(call, [&] {
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray) {
                auto* coords = args[1];
                auto* array_idx = args[2];

                auto* coords_ty = coords->Type()->As<core::type::Vector>();
                TINT_ASSERT(coords_ty);

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
                TINT_ASSERT(comp);

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
                        TINT_UNREACHABLE();
                }
            }

            tex = args[idx++];

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_ASSERT(tex_type);

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
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[idx++]))->Result());
                    offset_idx = is_depth ? 4 : 5;
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[idx++]))->Result());
                    break;
                default:
                    TINT_UNREACHABLE();
            }
            if (offset_idx > 0 && args.Length() > offset_idx) {
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
            TINT_ASSERT(tex_type);

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_UNREACHABLE();
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
            TINT_ASSERT(tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k1d:
                case core::type::TextureDimension::k2d:
                    params.Push(coords);

                    if (args.Length() > 3) {
                        params.Push(args[3]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    if (args.Length() > 3) {
                        params.Push(args[3]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    break;
                default:
                    TINT_UNREACHABLE();
            }

            core::ir::Instruction* result = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.vec4<f32>(), hlsl::BuiltinFn::kSample, tex, params);
            if (tex_type->Is<core::type::DepthTexture>()) {
                // Swizzle x from vec4 result for depth textures
                TINT_ASSERT(call->Result()->Type()->Is<core::type::F32>());
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
            TINT_ASSERT(tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // bias

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);

                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_UNREACHABLE();
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
            TINT_ASSERT(tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // depth ref

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);

                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_UNREACHABLE();
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
            TINT_ASSERT(tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // ddx
                    params.Push(args[4]);  // ddy

                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    params.Push(args[5]);

                    if (args.Length() > 6) {
                        params.Push(args[6]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);
                    params.Push(args[4]);

                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    params.Push(args[5]);
                    break;
                default:
                    TINT_UNREACHABLE();
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
            TINT_ASSERT(tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[3]));  // Level

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[4]));  // Level
                    if (args.Length() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[3]));  // Level

                    if (args.Length() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4<f32>(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[4]));  // Level
                    break;
                default:
                    TINT_UNREACHABLE();
            }

            core::ir::Instruction* result = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.vec4<f32>(), hlsl::BuiltinFn::kSampleLevel, tex, params);
            if (tex_type->Is<core::type::DepthTexture>()) {
                // Swizzle x from vec4 result for depth textures
                TINT_ASSERT(call->Result()->Type()->Is<core::type::F32>());
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
                b.Call<hlsl::ir::BuiltinCall>(ty.vec2<u32>(), hlsl::BuiltinFn::kF32Tof16, args[0]);

            auto* lower = b.Swizzle(ty.u32(), bc, {0});
            auto* upper = b.ShiftLeft(ty.u32(), b.Swizzle(ty.u32(), bc, {1}), 16_u);
            auto* res = b.Or(ty.u32(), lower, upper);
            call->Result()->ReplaceAllUsesWith(res->Result());
        });
        call->Destroy();
    }

    void Unpack2x16Float(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* x = b.And(ty.u32(), args[0], 0xffff_u);
            auto* y = b.ShiftRight(ty.u32(), args[0], 16_u);
            auto* conv = b.Construct(ty.vec2<u32>(), x, y);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kF16Tof32, conv);
        });
        call->Destroy();
    }

    void Pack2x16Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec2<f32>(), -1_f);
            auto* clamp_upper = b.Splat(ty.vec2<f32>(), 1_f);
            auto* clamp =
                b.Call(ty.vec2<f32>(), core::BuiltinFn::kClamp, args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(ty.vec2<f32>(), clamp, 32767_f);
            auto* round = b.Call(ty.vec2<f32>(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec2<i32>(), round);
            auto* res = b.And(ty.vec2<i32>(), conv, b.Splat(ty.vec2<i32>(), 0xffff_i));

            auto* lower = b.Swizzle(ty.i32(), res, {0});
            auto* upper = b.ShiftLeft(ty.i32(), b.Swizzle(ty.i32(), res, {1}), 16_u);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(), hlsl::BuiltinFn::kAsuint,
                                                    b.Or(ty.i32(), lower, upper));
        });
        call->Destroy();
    }

    void Unpack2x16Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* conv = b.Convert(ty.i32(), args[0]);
            auto* x = b.ShiftLeft(ty.i32(), conv, 16_u);

            auto* vec = b.Construct(ty.vec2<i32>(), x, conv);
            auto* v = b.ShiftRight(ty.vec2<i32>(), vec, b.Composite(ty.vec2<u32>(), 16_u));

            auto* flt = b.Convert(ty.vec2<f32>(), v);
            auto* scale = b.Divide(ty.vec2<f32>(), flt, 32767_f);

            auto* lower = b.Splat(ty.vec2<f32>(), -1_f);
            auto* upper = b.Splat(ty.vec2<f32>(), 1_f);
            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kClamp, scale, lower, upper);
        });
        call->Destroy();
    }

    void Pack2x16Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec2<f32>(), 0_f);
            auto* clamp_upper = b.Splat(ty.vec2<f32>(), 1_f);
            auto* clamp =
                b.Call(ty.vec2<f32>(), core::BuiltinFn::kClamp, args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(ty.vec2<f32>(), clamp, 65535_f);
            auto* round = b.Call(ty.vec2<f32>(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec2<u32>(), round);
            auto* lower = b.Swizzle(ty.u32(), conv, {0});
            auto* upper = b.ShiftLeft(ty.u32(), b.Swizzle(ty.u32(), conv, {1}), 16_u);
            auto* result = b.Or(ty.u32(), lower, upper);
            call->Result()->ReplaceAllUsesWith(result->Result());
        });
        call->Destroy();
    }

    void Unpack2x16Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* x = b.And(ty.u32(), args[0], 0xffff_u);
            auto* y = b.ShiftRight(ty.u32(), args[0], 16_u);
            auto* conv = b.Construct(ty.vec2<u32>(), x, y);
            auto* flt_conv = b.Convert(ty.vec2<f32>(), conv);
            auto* scale = b.Divide(ty.vec2<f32>(), flt_conv, 0xffff_f);

            call->Result()->ReplaceAllUsesWith(scale->Result());
        });
        call->Destroy();
    }

    void Pack4x8Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec4<f32>(), -1_f);
            auto* clamp_upper = b.Splat(ty.vec4<f32>(), 1_f);
            auto* clamp =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kClamp, args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(ty.vec4<f32>(), clamp, 127_f);
            auto* round = b.Call(ty.vec4<f32>(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec4<i32>(), round);
            auto* band = b.And(ty.vec4<i32>(), conv, b.Splat(ty.vec4<i32>(), 0xff_i));
            auto* x = b.Swizzle(ty.i32(), band, {0});
            auto* y = b.ShiftLeft(ty.i32(), b.Swizzle(ty.i32(), band, {1}), 8_u);
            auto* z = b.ShiftLeft(ty.i32(), b.Swizzle(ty.i32(), band, {2}), 16_u);
            auto* w = b.ShiftLeft(ty.i32(), b.Swizzle(ty.i32(), band, {3}), 24_u);
            b.CallWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kAsuint,
                b.Or(ty.i32(), x, b.Or(ty.i32(), y, b.Or(ty.i32(), z, w))));
        });
        call->Destroy();
    }

    void Unpack4x8Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* conv = b.Convert(ty.i32(), args[0]);
            auto* x = b.ShiftLeft(ty.i32(), conv, 24_u);
            auto* y = b.ShiftLeft(ty.i32(), conv, 16_u);
            auto* z = b.ShiftLeft(ty.i32(), conv, 8_u);
            auto* cons = b.Construct(ty.vec4<i32>(), x, y, z, conv);
            auto* shr = b.ShiftRight(ty.vec4<i32>(), cons, b.Composite(ty.vec4<u32>(), 24_u));
            auto* flt = b.Convert(ty.vec4<f32>(), shr);
            auto* scale = b.Divide(ty.vec4<f32>(), flt, 127_f);

            auto* lower = b.Splat(ty.vec4<f32>(), -1_f);
            auto* upper = b.Splat(ty.vec4<f32>(), 1_f);
            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kClamp, scale, lower, upper);
        });
        call->Destroy();
    }

    void Pack4x8Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec4<f32>(), 0_f);
            auto* clamp_upper = b.Splat(ty.vec4<f32>(), 1_f);
            auto* clamp =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kClamp, args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(ty.vec4<f32>(), clamp, 255_f);
            auto* round = b.Call(ty.vec4<f32>(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec4<u32>(), round);
            auto* x = b.Swizzle(ty.u32(), conv, {0});
            auto* y = b.ShiftLeft(ty.u32(), b.Swizzle(ty.u32(), conv, {1}), 8_u);
            auto* z = b.ShiftLeft(ty.u32(), b.Swizzle(ty.u32(), conv, {2}), 16_u);
            auto* w = b.ShiftLeft(ty.u32(), b.Swizzle(ty.u32(), conv, {3}), 24_u);
            auto* res = b.Or(ty.u32(), x, b.Or(ty.u32(), y, b.Or(ty.u32(), z, w)));

            call->Result()->ReplaceAllUsesWith(res->Result());
        });
        call->Destroy();
    }

    void Unpack4x8Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* val = args[0];
            auto* x = b.And(ty.u32(), val, 0xff_u);
            auto* y = b.And(ty.u32(), b.ShiftRight(ty.u32(), val, 8_u), 0xff_u);
            auto* z = b.And(ty.u32(), b.ShiftRight(ty.u32(), val, 16_u), 0xff_u);
            auto* w = b.ShiftRight(ty.u32(), val, 24_u);
            auto* cons = b.Construct(ty.vec4<u32>(), x, y, z, w);
            auto* conv = b.Convert(ty.vec4<f32>(), cons);
            auto* scale = b.Divide(ty.vec4<f32>(), conv, 255_f);

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
            auto* conv = b.CallExplicit<hlsl::ir::BuiltinCall>(type, hlsl::BuiltinFn::kConvert,
                                                               Vector{type}, args[0]);

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
            auto* conv = b.CallExplicit<hlsl::ir::BuiltinCall>(type, hlsl::BuiltinFn::kConvert,
                                                               Vector{type}, args[0]);

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
            fract = b.Multiply(arg_ty, arg_sign, fract);
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
        TINT_ASSERT(call->Args().Length() == 1);
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
        TINT_ASSERT(call->Args().Length() == 2);

        b.InsertBefore(call, [&] {
            auto* id = b.Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneIndex);
            auto* arg2 = call->Args()[1];

            core::ir::Instruction* inst = nullptr;
            switch (call->Func()) {
                case core::BuiltinFn::kSubgroupShuffleXor:
                    inst = b.Xor(ty.u32(), id, arg2);
                    break;
                case core::BuiltinFn::kSubgroupShuffleUp:
                    inst = b.Subtract(ty.u32(), id, arg2);
                    break;
                case core::BuiltinFn::kSubgroupShuffleDown:
                    inst = b.Add(ty.u32(), id, arg2);
                    break;
                default:
                    TINT_UNREACHABLE();
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
        TINT_ASSERT(call->Args().Length() == 1);
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
                    TINT_UNREACHABLE();
            }

            auto* arg1 = call->Args()[0];
            auto call_type = arg1->Type();
            auto* exclusive_call = b.Call<core::ir::CoreBuiltinCall>(call_type, builtin_sel, arg1);

            core::ir::Instruction* inst = nullptr;
            switch (call->Func()) {
                case core::BuiltinFn::kSubgroupInclusiveAdd:
                    inst = b.Add(call_type, exclusive_call, arg1);
                    break;
                case core::BuiltinFn::kSubgroupInclusiveMul:
                    inst = b.Multiply(call_type, exclusive_call, arg1);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
            call->Result()->ReplaceAllUsesWith(inst->Result());
        });
        call->Destroy();
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.BuiltinPolyfill",
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
