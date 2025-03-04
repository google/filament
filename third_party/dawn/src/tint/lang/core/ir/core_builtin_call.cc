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

#include "src/tint/lang/core/ir/core_builtin_call.h"

#include <utility>

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::CoreBuiltinCall);

namespace tint::core::ir {

CoreBuiltinCall::CoreBuiltinCall(Id id) : Base(id) {}

CoreBuiltinCall::CoreBuiltinCall(Id id,
                                 InstructionResult* result,
                                 core::BuiltinFn func,
                                 VectorRef<Value*> arguments)
    : Base(id, result, arguments), func_(func) {
    TINT_ASSERT(func != core::BuiltinFn::kNone);
}

CoreBuiltinCall::~CoreBuiltinCall() = default;

CoreBuiltinCall* CoreBuiltinCall::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result(0));
    auto args = ctx.Remap<CoreBuiltinCall::kDefaultNumOperands>(Args());
    return ctx.ir.CreateInstruction<CoreBuiltinCall>(new_result, func_, args);
}

tint::core::ir::Instruction::Accesses CoreBuiltinCall::GetSideEffects() const {
    switch (func_) {
        case BuiltinFn::kAtomicLoad:
        case BuiltinFn::kInputAttachmentLoad:
        case BuiltinFn::kSubgroupMatrixLoad:
        case BuiltinFn::kTextureSample:
        case BuiltinFn::kTextureSampleBias:
        case BuiltinFn::kTextureSampleCompare:
        case BuiltinFn::kTextureSampleCompareLevel:
        case BuiltinFn::kTextureSampleGrad:
        case BuiltinFn::kTextureSampleLevel:
        case BuiltinFn::kTextureSampleBaseClampToEdge:
        case BuiltinFn::kTextureLoad:
            return Accesses{Access::kLoad};

        case BuiltinFn::kSubgroupMatrixStore:
        case BuiltinFn::kTextureStore:
            return Accesses{Access::kStore};

        case BuiltinFn::kAtomicStore:
        case BuiltinFn::kAtomicAdd:
        case BuiltinFn::kAtomicSub:
        case BuiltinFn::kAtomicMax:
        case BuiltinFn::kAtomicMin:
        case BuiltinFn::kAtomicAnd:
        case BuiltinFn::kAtomicOr:
        case BuiltinFn::kAtomicXor:
        case BuiltinFn::kAtomicExchange:
        case BuiltinFn::kAtomicCompareExchangeWeak:
        case BuiltinFn::kDpdx:
        case BuiltinFn::kDpdxCoarse:
        case BuiltinFn::kDpdxFine:
        case BuiltinFn::kDpdy:
        case BuiltinFn::kDpdyCoarse:
        case BuiltinFn::kDpdyFine:
        case BuiltinFn::kFwidth:
        case BuiltinFn::kFwidthCoarse:
        case BuiltinFn::kFwidthFine:
        case BuiltinFn::kSubgroupBallot:
        case BuiltinFn::kSubgroupElect:
        case BuiltinFn::kSubgroupBroadcast:
        case BuiltinFn::kSubgroupBroadcastFirst:
        case BuiltinFn::kSubgroupShuffle:
        case BuiltinFn::kSubgroupShuffleXor:
        case BuiltinFn::kSubgroupShuffleUp:
        case BuiltinFn::kSubgroupShuffleDown:
        case BuiltinFn::kSubgroupAdd:
        case BuiltinFn::kSubgroupInclusiveAdd:
        case BuiltinFn::kSubgroupExclusiveAdd:
        case BuiltinFn::kSubgroupMul:
        case BuiltinFn::kSubgroupInclusiveMul:
        case BuiltinFn::kSubgroupExclusiveMul:
        case BuiltinFn::kSubgroupAnd:
        case BuiltinFn::kSubgroupOr:
        case BuiltinFn::kSubgroupXor:
        case BuiltinFn::kSubgroupMin:
        case BuiltinFn::kSubgroupMax:
        case BuiltinFn::kSubgroupAll:
        case BuiltinFn::kSubgroupAny:
        case BuiltinFn::kQuadBroadcast:
        case BuiltinFn::kQuadSwapX:
        case BuiltinFn::kQuadSwapY:
        case BuiltinFn::kQuadSwapDiagonal:
        case BuiltinFn::kStorageBarrier:
        case BuiltinFn::kWorkgroupBarrier:
        case BuiltinFn::kTextureBarrier:
            return Accesses{Access::kLoad, Access::kStore};

        case BuiltinFn::kAbs:
        case BuiltinFn::kAcos:
        case BuiltinFn::kAcosh:
        case BuiltinFn::kAll:
        case BuiltinFn::kAny:
        case BuiltinFn::kArrayLength:
        case BuiltinFn::kAsin:
        case BuiltinFn::kAsinh:
        case BuiltinFn::kAtan:
        case BuiltinFn::kAtan2:
        case BuiltinFn::kAtanh:
        case BuiltinFn::kCeil:
        case BuiltinFn::kClamp:
        case BuiltinFn::kCos:
        case BuiltinFn::kCosh:
        case BuiltinFn::kCountLeadingZeros:
        case BuiltinFn::kCountOneBits:
        case BuiltinFn::kCountTrailingZeros:
        case BuiltinFn::kCross:
        case BuiltinFn::kDegrees:
        case BuiltinFn::kDeterminant:
        case BuiltinFn::kDistance:
        case BuiltinFn::kDot:
        case BuiltinFn::kDot4I8Packed:
        case BuiltinFn::kDot4U8Packed:
        case BuiltinFn::kExp:
        case BuiltinFn::kExp2:
        case BuiltinFn::kExtractBits:
        case BuiltinFn::kFaceForward:
        case BuiltinFn::kFirstLeadingBit:
        case BuiltinFn::kFirstTrailingBit:
        case BuiltinFn::kFloor:
        case BuiltinFn::kFma:
        case BuiltinFn::kFract:
        case BuiltinFn::kFrexp:
        case BuiltinFn::kInsertBits:
        case BuiltinFn::kInverseSqrt:
        case BuiltinFn::kLdexp:
        case BuiltinFn::kLength:
        case BuiltinFn::kLog:
        case BuiltinFn::kLog2:
        case BuiltinFn::kMax:
        case BuiltinFn::kMin:
        case BuiltinFn::kMix:
        case BuiltinFn::kModf:
        case BuiltinFn::kNormalize:
        case BuiltinFn::kPack2X16Float:
        case BuiltinFn::kPack2X16Snorm:
        case BuiltinFn::kPack2X16Unorm:
        case BuiltinFn::kPack4X8Snorm:
        case BuiltinFn::kPack4X8Unorm:
        case BuiltinFn::kPack4XI8:
        case BuiltinFn::kPack4XU8:
        case BuiltinFn::kPack4XI8Clamp:
        case BuiltinFn::kPack4XU8Clamp:
        case BuiltinFn::kPow:
        case BuiltinFn::kQuantizeToF16:
        case BuiltinFn::kRadians:
        case BuiltinFn::kReflect:
        case BuiltinFn::kRefract:
        case BuiltinFn::kReverseBits:
        case BuiltinFn::kRound:
        case BuiltinFn::kSaturate:
        case BuiltinFn::kSelect:
        case BuiltinFn::kSign:
        case BuiltinFn::kSin:
        case BuiltinFn::kSinh:
        case BuiltinFn::kSmoothstep:
        case BuiltinFn::kSqrt:
        case BuiltinFn::kStep:
        case BuiltinFn::kTan:
        case BuiltinFn::kTanh:
        case BuiltinFn::kTextureDimensions:
        case BuiltinFn::kTextureGather:
        case BuiltinFn::kTextureGatherCompare:
        case BuiltinFn::kTextureNumLayers:
        case BuiltinFn::kTextureNumLevels:
        case BuiltinFn::kTextureNumSamples:
        case BuiltinFn::kTranspose:
        case BuiltinFn::kTrunc:
        case BuiltinFn::kUnpack2X16Float:
        case BuiltinFn::kUnpack2X16Snorm:
        case BuiltinFn::kUnpack2X16Unorm:
        case BuiltinFn::kUnpack4X8Snorm:
        case BuiltinFn::kUnpack4X8Unorm:
        case BuiltinFn::kUnpack4XI8:
        case BuiltinFn::kUnpack4XU8:
        case BuiltinFn::kSubgroupMatrixMultiply:
        case BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
        case BuiltinFn::kNone:
            break;
    }
    return Accesses{};
}

}  // namespace tint::core::ir
