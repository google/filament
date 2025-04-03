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

#include "src/tint/lang/wgsl/writer/raise/raise.h"

#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/writer/raise/ptr_to_ref.h"
#include "src/tint/lang/wgsl/writer/raise/value_to_let.h"

namespace tint::wgsl::writer {
namespace {

wgsl::BuiltinFn Convert(core::BuiltinFn fn) {
#define CASE(NAME)              \
    case core::BuiltinFn::NAME: \
        return wgsl::BuiltinFn::NAME;

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
        CASE(kSubgroupAdd)
        CASE(kSubgroupInclusiveAdd)
        CASE(kSubgroupExclusiveAdd)
        CASE(kSubgroupMul)
        CASE(kSubgroupInclusiveMul)
        CASE(kSubgroupExclusiveMul)
        CASE(kInputAttachmentLoad)
        CASE(kSubgroupAnd)
        CASE(kSubgroupOr)
        CASE(kSubgroupXor)
        CASE(kSubgroupMin)
        CASE(kSubgroupMax)
        CASE(kSubgroupAny)
        CASE(kSubgroupAll)
        CASE(kQuadBroadcast)
        CASE(kQuadSwapX)
        CASE(kQuadSwapY)
        CASE(kQuadSwapDiagonal)
        CASE(kSubgroupMatrixLoad)
        CASE(kSubgroupMatrixStore)
        CASE(kSubgroupMatrixMultiply)
        CASE(kSubgroupMatrixMultiplyAccumulate)
        case core::BuiltinFn::kNone:
            break;
    }
    TINT_ICE() << "unhandled builtin function: " << fn;
}

void ReplaceBuiltinFnCall(core::ir::Builder& b, core::ir::CoreBuiltinCall* call) {
    Vector<core::ir::Value*, 8> args(call->Args());
    auto* replacement = b.CallWithResult<wgsl::ir::BuiltinCall>(
        call->DetachResult(), Convert(call->Func()), std::move(args));
    if (!call->ExplicitTemplateParams().IsEmpty()) {
        Vector<const core::type::Type*, 4> tmpl_args;
        for (auto p : call->ExplicitTemplateParams()) {
            tmpl_args.Push(p);
        }
        replacement->SetExplicitTemplateParams(std::move(tmpl_args));
    }
    call->ReplaceWith(replacement);
    call->Destroy();
}

void ReplaceWorkgroupBarrier(core::ir::Builder& b, core::ir::CoreBuiltinCall* call) {
    // Pattern match:
    //    call workgroupBarrier
    //    %value = load &ptr
    //    call workgroupBarrier
    // And replace with:
    //    %value = call workgroupUniformLoad %ptr

    auto* load = As<core::ir::Load>(call->next.Get());
    if (!load || load->From()->Type()->As<core::type::Pointer>()->AddressSpace() !=
                     core::AddressSpace::kWorkgroup) {
        // No match
        ReplaceBuiltinFnCall(b, call);
        return;
    }

    auto* post_load = As<core::ir::CoreBuiltinCall>(load->next.Get());
    if (!post_load || post_load->Func() != core::BuiltinFn::kWorkgroupBarrier) {
        // No match
        ReplaceBuiltinFnCall(b, call);
        return;
    }

    // Remove both calls to workgroupBarrier
    post_load->Destroy();
    call->Destroy();

    // Replace load with workgroupUniformLoad
    auto* replacement = b.CallWithResult<wgsl::ir::BuiltinCall>(
        load->DetachResult(), wgsl::BuiltinFn::kWorkgroupUniformLoad, Vector{load->From()});
    load->ReplaceWith(replacement);
    load->Destroy();
}

}  // namespace

Result<SuccessType> Raise(core::ir::Module& mod) {
    core::ir::Builder b{mod};
    for (auto* inst : mod.Instructions()) {
        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
            switch (call->Func()) {
                case core::BuiltinFn::kWorkgroupBarrier:
                    ReplaceWorkgroupBarrier(b, call);
                    break;
                default:
                    ReplaceBuiltinFnCall(b, call);
                    break;
            }
        }
    }

    if (auto result = core::ir::transform::RenameConflicts(mod); result != Success) {
        return result.Failure();
    }
    if (auto result = raise::ValueToLet(mod); result != Success) {
        return result.Failure();
    }
    if (auto result = raise::PtrToRef(mod); result != Success) {
        return result.Failure();
    }

    return Success;
}

}  // namespace tint::wgsl::writer
