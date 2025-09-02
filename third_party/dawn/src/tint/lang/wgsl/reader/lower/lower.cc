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

#include "src/tint/lang/wgsl/reader/lower/lower.h"

#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::wgsl::reader {
namespace {

core::BuiltinFn Convert(wgsl::BuiltinFn fn) {
#define CASE(NAME)              \
    case wgsl::BuiltinFn::NAME: \
        return core::BuiltinFn::NAME;

    switch (fn) {
        CASE(kAbs)
        CASE(kAcos)
        CASE(kAcosh)
        CASE(kAll)
        CASE(kAny)
        CASE(kArrayLength)
        CASE(kAsin)
        CASE(kAsinh)
        CASE(kAtan)
        CASE(kAtan2)
        CASE(kAtanh)
        CASE(kCeil)
        CASE(kClamp)
        CASE(kCos)
        CASE(kCosh)
        CASE(kCountLeadingZeros)
        CASE(kCountOneBits)
        CASE(kCountTrailingZeros)
        CASE(kCross)
        CASE(kDegrees)
        CASE(kDeterminant)
        CASE(kDistance)
        CASE(kDot)
        CASE(kDot4I8Packed)
        CASE(kDot4U8Packed)
        CASE(kDpdx)
        CASE(kDpdxCoarse)
        CASE(kDpdxFine)
        CASE(kDpdy)
        CASE(kDpdyCoarse)
        CASE(kDpdyFine)
        CASE(kExp)
        CASE(kExp2)
        CASE(kExtractBits)
        CASE(kFaceForward)
        CASE(kFirstLeadingBit)
        CASE(kFirstTrailingBit)
        CASE(kFloor)
        CASE(kFma)
        CASE(kFract)
        CASE(kFrexp)
        CASE(kFwidth)
        CASE(kFwidthCoarse)
        CASE(kFwidthFine)
        CASE(kInsertBits)
        CASE(kInverseSqrt)
        CASE(kLdexp)
        CASE(kLength)
        CASE(kLog)
        CASE(kLog2)
        CASE(kMax)
        CASE(kMin)
        CASE(kMix)
        CASE(kModf)
        CASE(kNormalize)
        CASE(kPack2X16Float)
        CASE(kPack2X16Snorm)
        CASE(kPack2X16Unorm)
        CASE(kPack4X8Snorm)
        CASE(kPack4X8Unorm)
        CASE(kPack4XI8)
        CASE(kPack4XU8)
        CASE(kPack4XI8Clamp)
        CASE(kPack4XU8Clamp)
        CASE(kPow)
        CASE(kQuantizeToF16)
        CASE(kRadians)
        CASE(kReflect)
        CASE(kRefract)
        CASE(kReverseBits)
        CASE(kRound)
        CASE(kSaturate)
        CASE(kSelect)
        CASE(kSign)
        CASE(kSin)
        CASE(kSinh)
        CASE(kSmoothstep)
        CASE(kSqrt)
        CASE(kStep)
        CASE(kStorageBarrier)
        CASE(kTan)
        CASE(kTanh)
        CASE(kTranspose)
        CASE(kTrunc)
        CASE(kUnpack2X16Float)
        CASE(kUnpack2X16Snorm)
        CASE(kUnpack2X16Unorm)
        CASE(kUnpack4X8Snorm)
        CASE(kUnpack4X8Unorm)
        CASE(kUnpack4XI8)
        CASE(kUnpack4XU8)
        CASE(kWorkgroupBarrier)
        CASE(kTextureBarrier)
        CASE(kTextureDimensions)
        CASE(kTextureGather)
        CASE(kTextureGatherCompare)
        CASE(kTextureNumLayers)
        CASE(kTextureNumLevels)
        CASE(kTextureNumSamples)
        CASE(kTextureSample)
        CASE(kTextureSampleBias)
        CASE(kTextureSampleCompare)
        CASE(kTextureSampleCompareLevel)
        CASE(kTextureSampleGrad)
        CASE(kTextureSampleLevel)
        CASE(kTextureSampleBaseClampToEdge)
        CASE(kTextureStore)
        CASE(kTextureLoad)
        CASE(kAtomicLoad)
        CASE(kAtomicStore)
        CASE(kAtomicAdd)
        CASE(kAtomicSub)
        CASE(kAtomicMax)
        CASE(kAtomicMin)
        CASE(kAtomicAnd)
        CASE(kAtomicOr)
        CASE(kAtomicXor)
        CASE(kAtomicExchange)
        CASE(kAtomicCompareExchangeWeak)
        CASE(kSubgroupBallot)
        CASE(kSubgroupElect)
        CASE(kSubgroupBroadcast)
        CASE(kSubgroupBroadcastFirst)
        CASE(kSubgroupShuffle)
        CASE(kSubgroupShuffleXor)
        CASE(kSubgroupShuffleUp)
        CASE(kSubgroupShuffleDown)
        CASE(kInputAttachmentLoad)
        CASE(kSubgroupAdd)
        CASE(kSubgroupInclusiveAdd)
        CASE(kSubgroupExclusiveAdd)
        CASE(kSubgroupMul)
        CASE(kSubgroupInclusiveMul)
        CASE(kSubgroupExclusiveMul)
        CASE(kSubgroupAnd)
        CASE(kSubgroupOr)
        CASE(kSubgroupXor)
        CASE(kSubgroupMin)
        CASE(kSubgroupMax)
        CASE(kSubgroupAll)
        CASE(kSubgroupAny)
        CASE(kQuadBroadcast)
        CASE(kQuadSwapX)
        CASE(kQuadSwapY)
        CASE(kQuadSwapDiagonal)
        CASE(kSubgroupMatrixLoad)
        CASE(kSubgroupMatrixStore)
        CASE(kSubgroupMatrixMultiply)
        CASE(kSubgroupMatrixMultiplyAccumulate)
        CASE(kPrint)

        case tint::wgsl::BuiltinFn::kBitcast:               // should lower to ir::Bitcast
        case tint::wgsl::BuiltinFn::kWorkgroupUniformLoad:  // should be handled in Lower()
        case tint::wgsl::BuiltinFn::kTintMaterialize:
        case tint::wgsl::BuiltinFn::kNone:
            break;
    }
    TINT_ICE() << "unhandled builtin function: " << fn;
}

}  // namespace

Result<SuccessType> Lower(core::ir::Module& mod) {
    auto res =
        core::ir::ValidateAndDumpIfNeeded(mod, "wgsl.Lower",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (res != Success) {
        return res.Failure();
    }

    core::ir::Builder b{mod};
    core::type::Manager& ty{mod.Types()};
    for (auto* inst : mod.Instructions()) {
        if (auto* call = inst->As<wgsl::ir::BuiltinCall>()) {
            switch (call->Func()) {
                case BuiltinFn::kWorkgroupUniformLoad: {
                    auto* param0 = call->Args()[0];
                    TINT_ASSERT(param0->Type()->Is<core::type::Pointer>());
                    auto* storeType = param0->Type()->As<core::type::Pointer>()->StoreType();
                    // Replace:
                    //    %value = call workgroupUniformLoad %ptr
                    // With:
                    //    call workgroupBarrier
                    //    %value = {load || atomicLoad} &ptr
                    //    call workgroupBarrier
                    b.InsertBefore(call, [&] {
                        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
                        if (storeType->Is<core::type::Atomic>()) {
                            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kAtomicLoad,
                                             param0);
                        } else {
                            b.LoadWithResult(call->DetachResult(), param0);
                        }
                        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
                    });
                    break;
                }
                default: {
                    Vector<core::ir::Value*, 8> args(call->Args());
                    auto* replacement = b.CallWithResult(call->DetachResult(),
                                                         Convert(call->Func()), std::move(args));
                    if (!call->ExplicitTemplateParams().IsEmpty()) {
                        replacement->SetExplicitTemplateParams(call->ExplicitTemplateParams());
                    }
                    call->ReplaceWith(replacement);
                    break;
                }
            }
            call->Destroy();
        }
    }
    return Success;
}

}  // namespace tint::wgsl::reader
