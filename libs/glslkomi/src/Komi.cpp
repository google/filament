/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glslkomi/Komi.h>
#include <intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <unordered_map>
#include "utils/Log.h"
#include <sstream>

namespace glslkomi {

using namespace glslang;

std::string operatorToString(TOperator op) {
    switch (op) {
        case EOpNull: return "EOpNull";
        case EOpSequence: return "EOpSequence";
        case EOpScope: return "EOpScope";
        case EOpLinkerObjects: return "EOpLinkerObjects";
        case EOpFunctionCall: return "EOpFunctionCall";
        case EOpFunction: return "EOpFunction";
        case EOpParameters: return "EOpParameters";
        case EOpSpirvInst: return "EOpSpirvInst";
        case EOpNegative: return "EOpNegative";
        case EOpLogicalNot: return "EOpLogicalNot";
        case EOpVectorLogicalNot: return "EOpVectorLogicalNot";
        case EOpBitwiseNot: return "EOpBitwiseNot";
        case EOpPostIncrement: return "EOpPostIncrement";
        case EOpPostDecrement: return "EOpPostDecrement";
        case EOpPreIncrement: return "EOpPreIncrement";
        case EOpPreDecrement: return "EOpPreDecrement";
        case EOpCopyObject: return "EOpCopyObject";
        case EOpDeclare: return "EOpDeclare";
        case EOpConvInt8ToBool: return "EOpConvInt8ToBool";
        case EOpConvUint8ToBool: return "EOpConvUint8ToBool";
        case EOpConvInt16ToBool: return "EOpConvInt16ToBool";
        case EOpConvUint16ToBool: return "EOpConvUint16ToBool";
        case EOpConvIntToBool: return "EOpConvIntToBool";
        case EOpConvUintToBool: return "EOpConvUintToBool";
        case EOpConvInt64ToBool: return "EOpConvInt64ToBool";
        case EOpConvUint64ToBool: return "EOpConvUint64ToBool";
        case EOpConvFloat16ToBool: return "EOpConvFloat16ToBool";
        case EOpConvFloatToBool: return "EOpConvFloatToBool";
        case EOpConvDoubleToBool: return "EOpConvDoubleToBool";
        case EOpConvBoolToInt8: return "EOpConvBoolToInt8";
        case EOpConvBoolToUint8: return "EOpConvBoolToUint8";
        case EOpConvBoolToInt16: return "EOpConvBoolToInt16";
        case EOpConvBoolToUint16: return "EOpConvBoolToUint16";
        case EOpConvBoolToInt: return "EOpConvBoolToInt";
        case EOpConvBoolToUint: return "EOpConvBoolToUint";
        case EOpConvBoolToInt64: return "EOpConvBoolToInt64";
        case EOpConvBoolToUint64: return "EOpConvBoolToUint64";
        case EOpConvBoolToFloat16: return "EOpConvBoolToFloat16";
        case EOpConvBoolToFloat: return "EOpConvBoolToFloat";
        case EOpConvBoolToDouble: return "EOpConvBoolToDouble";
        case EOpConvInt8ToInt16: return "EOpConvInt8ToInt16";
        case EOpConvInt8ToInt: return "EOpConvInt8ToInt";
        case EOpConvInt8ToInt64: return "EOpConvInt8ToInt64";
        case EOpConvInt8ToUint8: return "EOpConvInt8ToUint8";
        case EOpConvInt8ToUint16: return "EOpConvInt8ToUint16";
        case EOpConvInt8ToUint: return "EOpConvInt8ToUint";
        case EOpConvInt8ToUint64: return "EOpConvInt8ToUint64";
        case EOpConvUint8ToInt8: return "EOpConvUint8ToInt8";
        case EOpConvUint8ToInt16: return "EOpConvUint8ToInt16";
        case EOpConvUint8ToInt: return "EOpConvUint8ToInt";
        case EOpConvUint8ToInt64: return "EOpConvUint8ToInt64";
        case EOpConvUint8ToUint16: return "EOpConvUint8ToUint16";
        case EOpConvUint8ToUint: return "EOpConvUint8ToUint";
        case EOpConvUint8ToUint64: return "EOpConvUint8ToUint64";
        case EOpConvInt8ToFloat16: return "EOpConvInt8ToFloat16";
        case EOpConvInt8ToFloat: return "EOpConvInt8ToFloat";
        case EOpConvInt8ToDouble: return "EOpConvInt8ToDouble";
        case EOpConvUint8ToFloat16: return "EOpConvUint8ToFloat16";
        case EOpConvUint8ToFloat: return "EOpConvUint8ToFloat";
        case EOpConvUint8ToDouble: return "EOpConvUint8ToDouble";
        case EOpConvInt16ToInt8: return "EOpConvInt16ToInt8";
        case EOpConvInt16ToInt: return "EOpConvInt16ToInt";
        case EOpConvInt16ToInt64: return "EOpConvInt16ToInt64";
        case EOpConvInt16ToUint8: return "EOpConvInt16ToUint8";
        case EOpConvInt16ToUint16: return "EOpConvInt16ToUint16";
        case EOpConvInt16ToUint: return "EOpConvInt16ToUint";
        case EOpConvInt16ToUint64: return "EOpConvInt16ToUint64";
        case EOpConvUint16ToInt8: return "EOpConvUint16ToInt8";
        case EOpConvUint16ToInt16: return "EOpConvUint16ToInt16";
        case EOpConvUint16ToInt: return "EOpConvUint16ToInt";
        case EOpConvUint16ToInt64: return "EOpConvUint16ToInt64";
        case EOpConvUint16ToUint8: return "EOpConvUint16ToUint8";
        case EOpConvUint16ToUint: return "EOpConvUint16ToUint";
        case EOpConvUint16ToUint64: return "EOpConvUint16ToUint64";
        case EOpConvInt16ToFloat16: return "EOpConvInt16ToFloat16";
        case EOpConvInt16ToFloat: return "EOpConvInt16ToFloat";
        case EOpConvInt16ToDouble: return "EOpConvInt16ToDouble";
        case EOpConvUint16ToFloat16: return "EOpConvUint16ToFloat16";
        case EOpConvUint16ToFloat: return "EOpConvUint16ToFloat";
        case EOpConvUint16ToDouble: return "EOpConvUint16ToDouble";
        case EOpConvIntToInt8: return "EOpConvIntToInt8";
        case EOpConvIntToInt16: return "EOpConvIntToInt16";
        case EOpConvIntToInt64: return "EOpConvIntToInt64";
        case EOpConvIntToUint8: return "EOpConvIntToUint8";
        case EOpConvIntToUint16: return "EOpConvIntToUint16";
        case EOpConvIntToUint: return "EOpConvIntToUint";
        case EOpConvIntToUint64: return "EOpConvIntToUint64";
        case EOpConvUintToInt8: return "EOpConvUintToInt8";
        case EOpConvUintToInt16: return "EOpConvUintToInt16";
        case EOpConvUintToInt: return "EOpConvUintToInt";
        case EOpConvUintToInt64: return "EOpConvUintToInt64";
        case EOpConvUintToUint8: return "EOpConvUintToUint8";
        case EOpConvUintToUint16: return "EOpConvUintToUint16";
        case EOpConvUintToUint64: return "EOpConvUintToUint64";
        case EOpConvIntToFloat16: return "EOpConvIntToFloat16";
        case EOpConvIntToFloat: return "EOpConvIntToFloat";
        case EOpConvIntToDouble: return "EOpConvIntToDouble";
        case EOpConvUintToFloat16: return "EOpConvUintToFloat16";
        case EOpConvUintToFloat: return "EOpConvUintToFloat";
        case EOpConvUintToDouble: return "EOpConvUintToDouble";
        case EOpConvInt64ToInt8: return "EOpConvInt64ToInt8";
        case EOpConvInt64ToInt16: return "EOpConvInt64ToInt16";
        case EOpConvInt64ToInt: return "EOpConvInt64ToInt";
        case EOpConvInt64ToUint8: return "EOpConvInt64ToUint8";
        case EOpConvInt64ToUint16: return "EOpConvInt64ToUint16";
        case EOpConvInt64ToUint: return "EOpConvInt64ToUint";
        case EOpConvInt64ToUint64: return "EOpConvInt64ToUint64";
        case EOpConvUint64ToInt8: return "EOpConvUint64ToInt8";
        case EOpConvUint64ToInt16: return "EOpConvUint64ToInt16";
        case EOpConvUint64ToInt: return "EOpConvUint64ToInt";
        case EOpConvUint64ToInt64: return "EOpConvUint64ToInt64";
        case EOpConvUint64ToUint8: return "EOpConvUint64ToUint8";
        case EOpConvUint64ToUint16: return "EOpConvUint64ToUint16";
        case EOpConvUint64ToUint: return "EOpConvUint64ToUint";
        case EOpConvInt64ToFloat16: return "EOpConvInt64ToFloat16";
        case EOpConvInt64ToFloat: return "EOpConvInt64ToFloat";
        case EOpConvInt64ToDouble: return "EOpConvInt64ToDouble";
        case EOpConvUint64ToFloat16: return "EOpConvUint64ToFloat16";
        case EOpConvUint64ToFloat: return "EOpConvUint64ToFloat";
        case EOpConvUint64ToDouble: return "EOpConvUint64ToDouble";
        case EOpConvFloat16ToInt8: return "EOpConvFloat16ToInt8";
        case EOpConvFloat16ToInt16: return "EOpConvFloat16ToInt16";
        case EOpConvFloat16ToInt: return "EOpConvFloat16ToInt";
        case EOpConvFloat16ToInt64: return "EOpConvFloat16ToInt64";
        case EOpConvFloat16ToUint8: return "EOpConvFloat16ToUint8";
        case EOpConvFloat16ToUint16: return "EOpConvFloat16ToUint16";
        case EOpConvFloat16ToUint: return "EOpConvFloat16ToUint";
        case EOpConvFloat16ToUint64: return "EOpConvFloat16ToUint64";
        case EOpConvFloat16ToFloat: return "EOpConvFloat16ToFloat";
        case EOpConvFloat16ToDouble: return "EOpConvFloat16ToDouble";
        case EOpConvFloatToInt8: return "EOpConvFloatToInt8";
        case EOpConvFloatToInt16: return "EOpConvFloatToInt16";
        case EOpConvFloatToInt: return "EOpConvFloatToInt";
        case EOpConvFloatToInt64: return "EOpConvFloatToInt64";
        case EOpConvFloatToUint8: return "EOpConvFloatToUint8";
        case EOpConvFloatToUint16: return "EOpConvFloatToUint16";
        case EOpConvFloatToUint: return "EOpConvFloatToUint";
        case EOpConvFloatToUint64: return "EOpConvFloatToUint64";
        case EOpConvFloatToFloat16: return "EOpConvFloatToFloat16";
        case EOpConvFloatToDouble: return "EOpConvFloatToDouble";
        case EOpConvDoubleToInt8: return "EOpConvDoubleToInt8";
        case EOpConvDoubleToInt16: return "EOpConvDoubleToInt16";
        case EOpConvDoubleToInt: return "EOpConvDoubleToInt";
        case EOpConvDoubleToInt64: return "EOpConvDoubleToInt64";
        case EOpConvDoubleToUint8: return "EOpConvDoubleToUint8";
        case EOpConvDoubleToUint16: return "EOpConvDoubleToUint16";
        case EOpConvDoubleToUint: return "EOpConvDoubleToUint";
        case EOpConvDoubleToUint64: return "EOpConvDoubleToUint64";
        case EOpConvDoubleToFloat16: return "EOpConvDoubleToFloat16";
        case EOpConvDoubleToFloat: return "EOpConvDoubleToFloat";
        case EOpConvUint64ToPtr: return "EOpConvUint64ToPtr";
        case EOpConvPtrToUint64: return "EOpConvPtrToUint64";
        case EOpConvUvec2ToPtr: return "EOpConvUvec2ToPtr";
        case EOpConvPtrToUvec2: return "EOpConvPtrToUvec2";
        case EOpConvUint64ToAccStruct: return "EOpConvUint64ToAccStruct";
        case EOpConvUvec2ToAccStruct: return "EOpConvUvec2ToAccStruct";
        case EOpAdd: return "EOpAdd";
        case EOpSub: return "EOpSub";
        case EOpMul: return "EOpMul";
        case EOpDiv: return "EOpDiv";
        case EOpMod: return "EOpMod";
        case EOpRightShift: return "EOpRightShift";
        case EOpLeftShift: return "EOpLeftShift";
        case EOpAnd: return "EOpAnd";
        case EOpInclusiveOr: return "EOpInclusiveOr";
        case EOpExclusiveOr: return "EOpExclusiveOr";
        case EOpEqual: return "EOpEqual";
        case EOpNotEqual: return "EOpNotEqual";
        case EOpVectorEqual: return "EOpVectorEqual";
        case EOpVectorNotEqual: return "EOpVectorNotEqual";
        case EOpLessThan: return "EOpLessThan";
        case EOpGreaterThan: return "EOpGreaterThan";
        case EOpLessThanEqual: return "EOpLessThanEqual";
        case EOpGreaterThanEqual: return "EOpGreaterThanEqual";
        case EOpComma: return "EOpComma";
        case EOpVectorTimesScalar: return "EOpVectorTimesScalar";
        case EOpVectorTimesMatrix: return "EOpVectorTimesMatrix";
        case EOpMatrixTimesVector: return "EOpMatrixTimesVector";
        case EOpMatrixTimesScalar: return "EOpMatrixTimesScalar";
        case EOpLogicalOr: return "EOpLogicalOr";
        case EOpLogicalXor: return "EOpLogicalXor";
        case EOpLogicalAnd: return "EOpLogicalAnd";
        case EOpIndexDirect: return "EOpIndexDirect";
        case EOpIndexIndirect: return "EOpIndexIndirect";
        case EOpIndexDirectStruct: return "EOpIndexDirectStruct";
        case EOpVectorSwizzle: return "EOpVectorSwizzle";
        case EOpMethod: return "EOpMethod";
        case EOpScoping: return "EOpScoping";
        case EOpRadians: return "EOpRadians";
        case EOpDegrees: return "EOpDegrees";
        case EOpSin: return "EOpSin";
        case EOpCos: return "EOpCos";
        case EOpTan: return "EOpTan";
        case EOpAsin: return "EOpAsin";
        case EOpAcos: return "EOpAcos";
        case EOpAtan: return "EOpAtan";
        case EOpSinh: return "EOpSinh";
        case EOpCosh: return "EOpCosh";
        case EOpTanh: return "EOpTanh";
        case EOpAsinh: return "EOpAsinh";
        case EOpAcosh: return "EOpAcosh";
        case EOpAtanh: return "EOpAtanh";
        case EOpPow: return "EOpPow";
        case EOpExp: return "EOpExp";
        case EOpLog: return "EOpLog";
        case EOpExp2: return "EOpExp2";
        case EOpLog2: return "EOpLog2";
        case EOpSqrt: return "EOpSqrt";
        case EOpInverseSqrt: return "EOpInverseSqrt";
        case EOpAbs: return "EOpAbs";
        case EOpSign: return "EOpSign";
        case EOpFloor: return "EOpFloor";
        case EOpTrunc: return "EOpTrunc";
        case EOpRound: return "EOpRound";
        case EOpRoundEven: return "EOpRoundEven";
        case EOpCeil: return "EOpCeil";
        case EOpFract: return "EOpFract";
        case EOpModf: return "EOpModf";
        case EOpMin: return "EOpMin";
        case EOpMax: return "EOpMax";
        case EOpClamp: return "EOpClamp";
        case EOpMix: return "EOpMix";
        case EOpStep: return "EOpStep";
        case EOpSmoothStep: return "EOpSmoothStep";
        case EOpIsNan: return "EOpIsNan";
        case EOpIsInf: return "EOpIsInf";
        case EOpFma: return "EOpFma";
        case EOpFrexp: return "EOpFrexp";
        case EOpLdexp: return "EOpLdexp";
        case EOpFloatBitsToInt: return "EOpFloatBitsToInt";
        case EOpFloatBitsToUint: return "EOpFloatBitsToUint";
        case EOpIntBitsToFloat: return "EOpIntBitsToFloat";
        case EOpUintBitsToFloat: return "EOpUintBitsToFloat";
        case EOpDoubleBitsToInt64: return "EOpDoubleBitsToInt64";
        case EOpDoubleBitsToUint64: return "EOpDoubleBitsToUint64";
        case EOpInt64BitsToDouble: return "EOpInt64BitsToDouble";
        case EOpUint64BitsToDouble: return "EOpUint64BitsToDouble";
        case EOpFloat16BitsToInt16: return "EOpFloat16BitsToInt16";
        case EOpFloat16BitsToUint16: return "EOpFloat16BitsToUint16";
        case EOpInt16BitsToFloat16: return "EOpInt16BitsToFloat16";
        case EOpUint16BitsToFloat16: return "EOpUint16BitsToFloat16";
        case EOpPackSnorm2x16: return "EOpPackSnorm2x16";
        case EOpUnpackSnorm2x16: return "EOpUnpackSnorm2x16";
        case EOpPackUnorm2x16: return "EOpPackUnorm2x16";
        case EOpUnpackUnorm2x16: return "EOpUnpackUnorm2x16";
        case EOpPackSnorm4x8: return "EOpPackSnorm4x8";
        case EOpUnpackSnorm4x8: return "EOpUnpackSnorm4x8";
        case EOpPackUnorm4x8: return "EOpPackUnorm4x8";
        case EOpUnpackUnorm4x8: return "EOpUnpackUnorm4x8";
        case EOpPackHalf2x16: return "EOpPackHalf2x16";
        case EOpUnpackHalf2x16: return "EOpUnpackHalf2x16";
        case EOpPackDouble2x32: return "EOpPackDouble2x32";
        case EOpUnpackDouble2x32: return "EOpUnpackDouble2x32";
        case EOpPackInt2x32: return "EOpPackInt2x32";
        case EOpUnpackInt2x32: return "EOpUnpackInt2x32";
        case EOpPackUint2x32: return "EOpPackUint2x32";
        case EOpUnpackUint2x32: return "EOpUnpackUint2x32";
        case EOpPackFloat2x16: return "EOpPackFloat2x16";
        case EOpUnpackFloat2x16: return "EOpUnpackFloat2x16";
        case EOpPackInt2x16: return "EOpPackInt2x16";
        case EOpUnpackInt2x16: return "EOpUnpackInt2x16";
        case EOpPackUint2x16: return "EOpPackUint2x16";
        case EOpUnpackUint2x16: return "EOpUnpackUint2x16";
        case EOpPackInt4x16: return "EOpPackInt4x16";
        case EOpUnpackInt4x16: return "EOpUnpackInt4x16";
        case EOpPackUint4x16: return "EOpPackUint4x16";
        case EOpUnpackUint4x16: return "EOpUnpackUint4x16";
        case EOpPack16: return "EOpPack16";
        case EOpPack32: return "EOpPack32";
        case EOpPack64: return "EOpPack64";
        case EOpUnpack32: return "EOpUnpack32";
        case EOpUnpack16: return "EOpUnpack16";
        case EOpUnpack8: return "EOpUnpack8";
        case EOpLength: return "EOpLength";
        case EOpDistance: return "EOpDistance";
        case EOpDot: return "EOpDot";
        case EOpCross: return "EOpCross";
        case EOpNormalize: return "EOpNormalize";
        case EOpFaceForward: return "EOpFaceForward";
        case EOpReflect: return "EOpReflect";
        case EOpRefract: return "EOpRefract";
        case EOpMin3: return "EOpMin3";
        case EOpMax3: return "EOpMax3";
        case EOpMid3: return "EOpMid3";
        case EOpDPdx: return "EOpDPdx";
        case EOpDPdy: return "EOpDPdy";
        case EOpFwidth: return "EOpFwidth";
        case EOpDPdxFine: return "EOpDPdxFine";
        case EOpDPdyFine: return "EOpDPdyFine";
        case EOpFwidthFine: return "EOpFwidthFine";
        case EOpDPdxCoarse: return "EOpDPdxCoarse";
        case EOpDPdyCoarse: return "EOpDPdyCoarse";
        case EOpFwidthCoarse: return "EOpFwidthCoarse";
        case EOpInterpolateAtCentroid: return "EOpInterpolateAtCentroid";
        case EOpInterpolateAtSample: return "EOpInterpolateAtSample";
        case EOpInterpolateAtOffset: return "EOpInterpolateAtOffset";
        case EOpInterpolateAtVertex: return "EOpInterpolateAtVertex";
        case EOpMatrixTimesMatrix: return "EOpMatrixTimesMatrix";
        case EOpOuterProduct: return "EOpOuterProduct";
        case EOpDeterminant: return "EOpDeterminant";
        case EOpMatrixInverse: return "EOpMatrixInverse";
        case EOpTranspose: return "EOpTranspose";
        case EOpFtransform: return "EOpFtransform";
        case EOpNoise: return "EOpNoise";
        case EOpEmitVertex: return "EOpEmitVertex";
        case EOpEndPrimitive: return "EOpEndPrimitive";
        case EOpEmitStreamVertex: return "EOpEmitStreamVertex";
        case EOpEndStreamPrimitive: return "EOpEndStreamPrimitive";
        case EOpBarrier: return "EOpBarrier";
        case EOpMemoryBarrier: return "EOpMemoryBarrier";
        case EOpMemoryBarrierAtomicCounter: return "EOpMemoryBarrierAtomicCounter";
        case EOpMemoryBarrierBuffer: return "EOpMemoryBarrierBuffer";
        case EOpMemoryBarrierImage: return "EOpMemoryBarrierImage";
        case EOpMemoryBarrierShared: return "EOpMemoryBarrierShared";
        case EOpGroupMemoryBarrier: return "EOpGroupMemoryBarrier";
        case EOpBallot: return "EOpBallot";
        case EOpReadInvocation: return "EOpReadInvocation";
        case EOpReadFirstInvocation: return "EOpReadFirstInvocation";
        case EOpAnyInvocation: return "EOpAnyInvocation";
        case EOpAllInvocations: return "EOpAllInvocations";
        case EOpAllInvocationsEqual: return "EOpAllInvocationsEqual";
        case EOpSubgroupGuardStart: return "EOpSubgroupGuardStart";
        case EOpSubgroupBarrier: return "EOpSubgroupBarrier";
        case EOpSubgroupMemoryBarrier: return "EOpSubgroupMemoryBarrier";
        case EOpSubgroupMemoryBarrierBuffer: return "EOpSubgroupMemoryBarrierBuffer";
        case EOpSubgroupMemoryBarrierImage: return "EOpSubgroupMemoryBarrierImage";
        case EOpSubgroupMemoryBarrierShared: return "EOpSubgroupMemoryBarrierShared";
        case EOpSubgroupElect: return "EOpSubgroupElect";
        case EOpSubgroupAll: return "EOpSubgroupAll";
        case EOpSubgroupAny: return "EOpSubgroupAny";
        case EOpSubgroupAllEqual: return "EOpSubgroupAllEqual";
        case EOpSubgroupBroadcast: return "EOpSubgroupBroadcast";
        case EOpSubgroupBroadcastFirst: return "EOpSubgroupBroadcastFirst";
        case EOpSubgroupBallot: return "EOpSubgroupBallot";
        case EOpSubgroupInverseBallot: return "EOpSubgroupInverseBallot";
        case EOpSubgroupBallotBitExtract: return "EOpSubgroupBallotBitExtract";
        case EOpSubgroupBallotBitCount: return "EOpSubgroupBallotBitCount";
        case EOpSubgroupBallotInclusiveBitCount: return "EOpSubgroupBallotInclusiveBitCount";
        case EOpSubgroupBallotExclusiveBitCount: return "EOpSubgroupBallotExclusiveBitCount";
        case EOpSubgroupBallotFindLSB: return "EOpSubgroupBallotFindLSB";
        case EOpSubgroupBallotFindMSB: return "EOpSubgroupBallotFindMSB";
        case EOpSubgroupShuffle: return "EOpSubgroupShuffle";
        case EOpSubgroupShuffleXor: return "EOpSubgroupShuffleXor";
        case EOpSubgroupShuffleUp: return "EOpSubgroupShuffleUp";
        case EOpSubgroupShuffleDown: return "EOpSubgroupShuffleDown";
        case EOpSubgroupAdd: return "EOpSubgroupAdd";
        case EOpSubgroupMul: return "EOpSubgroupMul";
        case EOpSubgroupMin: return "EOpSubgroupMin";
        case EOpSubgroupMax: return "EOpSubgroupMax";
        case EOpSubgroupAnd: return "EOpSubgroupAnd";
        case EOpSubgroupOr: return "EOpSubgroupOr";
        case EOpSubgroupXor: return "EOpSubgroupXor";
        case EOpSubgroupInclusiveAdd: return "EOpSubgroupInclusiveAdd";
        case EOpSubgroupInclusiveMul: return "EOpSubgroupInclusiveMul";
        case EOpSubgroupInclusiveMin: return "EOpSubgroupInclusiveMin";
        case EOpSubgroupInclusiveMax: return "EOpSubgroupInclusiveMax";
        case EOpSubgroupInclusiveAnd: return "EOpSubgroupInclusiveAnd";
        case EOpSubgroupInclusiveOr: return "EOpSubgroupInclusiveOr";
        case EOpSubgroupInclusiveXor: return "EOpSubgroupInclusiveXor";
        case EOpSubgroupExclusiveAdd: return "EOpSubgroupExclusiveAdd";
        case EOpSubgroupExclusiveMul: return "EOpSubgroupExclusiveMul";
        case EOpSubgroupExclusiveMin: return "EOpSubgroupExclusiveMin";
        case EOpSubgroupExclusiveMax: return "EOpSubgroupExclusiveMax";
        case EOpSubgroupExclusiveAnd: return "EOpSubgroupExclusiveAnd";
        case EOpSubgroupExclusiveOr: return "EOpSubgroupExclusiveOr";
        case EOpSubgroupExclusiveXor: return "EOpSubgroupExclusiveXor";
        case EOpSubgroupClusteredAdd: return "EOpSubgroupClusteredAdd";
        case EOpSubgroupClusteredMul: return "EOpSubgroupClusteredMul";
        case EOpSubgroupClusteredMin: return "EOpSubgroupClusteredMin";
        case EOpSubgroupClusteredMax: return "EOpSubgroupClusteredMax";
        case EOpSubgroupClusteredAnd: return "EOpSubgroupClusteredAnd";
        case EOpSubgroupClusteredOr: return "EOpSubgroupClusteredOr";
        case EOpSubgroupClusteredXor: return "EOpSubgroupClusteredXor";
        case EOpSubgroupQuadBroadcast: return "EOpSubgroupQuadBroadcast";
        case EOpSubgroupQuadSwapHorizontal: return "EOpSubgroupQuadSwapHorizontal";
        case EOpSubgroupQuadSwapVertical: return "EOpSubgroupQuadSwapVertical";
        case EOpSubgroupQuadSwapDiagonal: return "EOpSubgroupQuadSwapDiagonal";
        case EOpSubgroupPartition: return "EOpSubgroupPartition";
        case EOpSubgroupPartitionedAdd: return "EOpSubgroupPartitionedAdd";
        case EOpSubgroupPartitionedMul: return "EOpSubgroupPartitionedMul";
        case EOpSubgroupPartitionedMin: return "EOpSubgroupPartitionedMin";
        case EOpSubgroupPartitionedMax: return "EOpSubgroupPartitionedMax";
        case EOpSubgroupPartitionedAnd: return "EOpSubgroupPartitionedAnd";
        case EOpSubgroupPartitionedOr: return "EOpSubgroupPartitionedOr";
        case EOpSubgroupPartitionedXor: return "EOpSubgroupPartitionedXor";
        case EOpSubgroupPartitionedInclusiveAdd: return "EOpSubgroupPartitionedInclusiveAdd";
        case EOpSubgroupPartitionedInclusiveMul: return "EOpSubgroupPartitionedInclusiveMul";
        case EOpSubgroupPartitionedInclusiveMin: return "EOpSubgroupPartitionedInclusiveMin";
        case EOpSubgroupPartitionedInclusiveMax: return "EOpSubgroupPartitionedInclusiveMax";
        case EOpSubgroupPartitionedInclusiveAnd: return "EOpSubgroupPartitionedInclusiveAnd";
        case EOpSubgroupPartitionedInclusiveOr: return "EOpSubgroupPartitionedInclusiveOr";
        case EOpSubgroupPartitionedInclusiveXor: return "EOpSubgroupPartitionedInclusiveXor";
        case EOpSubgroupPartitionedExclusiveAdd: return "EOpSubgroupPartitionedExclusiveAdd";
        case EOpSubgroupPartitionedExclusiveMul: return "EOpSubgroupPartitionedExclusiveMul";
        case EOpSubgroupPartitionedExclusiveMin: return "EOpSubgroupPartitionedExclusiveMin";
        case EOpSubgroupPartitionedExclusiveMax: return "EOpSubgroupPartitionedExclusiveMax";
        case EOpSubgroupPartitionedExclusiveAnd: return "EOpSubgroupPartitionedExclusiveAnd";
        case EOpSubgroupPartitionedExclusiveOr: return "EOpSubgroupPartitionedExclusiveOr";
        case EOpSubgroupPartitionedExclusiveXor: return "EOpSubgroupPartitionedExclusiveXor";
        case EOpSubgroupGuardStop: return "EOpSubgroupGuardStop";
        case EOpMinInvocations: return "EOpMinInvocations";
        case EOpMaxInvocations: return "EOpMaxInvocations";
        case EOpAddInvocations: return "EOpAddInvocations";
        case EOpMinInvocationsNonUniform: return "EOpMinInvocationsNonUniform";
        case EOpMaxInvocationsNonUniform: return "EOpMaxInvocationsNonUniform";
        case EOpAddInvocationsNonUniform: return "EOpAddInvocationsNonUniform";
        case EOpMinInvocationsInclusiveScan: return "EOpMinInvocationsInclusiveScan";
        case EOpMaxInvocationsInclusiveScan: return "EOpMaxInvocationsInclusiveScan";
        case EOpAddInvocationsInclusiveScan: return "EOpAddInvocationsInclusiveScan";
        case EOpMinInvocationsInclusiveScanNonUniform: return "EOpMinInvocationsInclusiveScanNonUniform";
        case EOpMaxInvocationsInclusiveScanNonUniform: return "EOpMaxInvocationsInclusiveScanNonUniform";
        case EOpAddInvocationsInclusiveScanNonUniform: return "EOpAddInvocationsInclusiveScanNonUniform";
        case EOpMinInvocationsExclusiveScan: return "EOpMinInvocationsExclusiveScan";
        case EOpMaxInvocationsExclusiveScan: return "EOpMaxInvocationsExclusiveScan";
        case EOpAddInvocationsExclusiveScan: return "EOpAddInvocationsExclusiveScan";
        case EOpMinInvocationsExclusiveScanNonUniform: return "EOpMinInvocationsExclusiveScanNonUniform";
        case EOpMaxInvocationsExclusiveScanNonUniform: return "EOpMaxInvocationsExclusiveScanNonUniform";
        case EOpAddInvocationsExclusiveScanNonUniform: return "EOpAddInvocationsExclusiveScanNonUniform";
        case EOpSwizzleInvocations: return "EOpSwizzleInvocations";
        case EOpSwizzleInvocationsMasked: return "EOpSwizzleInvocationsMasked";
        case EOpWriteInvocation: return "EOpWriteInvocation";
        case EOpMbcnt: return "EOpMbcnt";
        case EOpCubeFaceIndex: return "EOpCubeFaceIndex";
        case EOpCubeFaceCoord: return "EOpCubeFaceCoord";
        case EOpTime: return "EOpTime";
        case EOpAtomicAdd: return "EOpAtomicAdd";
        case EOpAtomicSubtract: return "EOpAtomicSubtract";
        case EOpAtomicMin: return "EOpAtomicMin";
        case EOpAtomicMax: return "EOpAtomicMax";
        case EOpAtomicAnd: return "EOpAtomicAnd";
        case EOpAtomicOr: return "EOpAtomicOr";
        case EOpAtomicXor: return "EOpAtomicXor";
        case EOpAtomicExchange: return "EOpAtomicExchange";
        case EOpAtomicCompSwap: return "EOpAtomicCompSwap";
        case EOpAtomicLoad: return "EOpAtomicLoad";
        case EOpAtomicStore: return "EOpAtomicStore";
        case EOpAtomicCounterIncrement: return "EOpAtomicCounterIncrement";
        case EOpAtomicCounterDecrement: return "EOpAtomicCounterDecrement";
        case EOpAtomicCounter: return "EOpAtomicCounter";
        case EOpAtomicCounterAdd: return "EOpAtomicCounterAdd";
        case EOpAtomicCounterSubtract: return "EOpAtomicCounterSubtract";
        case EOpAtomicCounterMin: return "EOpAtomicCounterMin";
        case EOpAtomicCounterMax: return "EOpAtomicCounterMax";
        case EOpAtomicCounterAnd: return "EOpAtomicCounterAnd";
        case EOpAtomicCounterOr: return "EOpAtomicCounterOr";
        case EOpAtomicCounterXor: return "EOpAtomicCounterXor";
        case EOpAtomicCounterExchange: return "EOpAtomicCounterExchange";
        case EOpAtomicCounterCompSwap: return "EOpAtomicCounterCompSwap";
        case EOpAny: return "EOpAny";
        case EOpAll: return "EOpAll";
        case EOpCooperativeMatrixLoad: return "EOpCooperativeMatrixLoad";
        case EOpCooperativeMatrixStore: return "EOpCooperativeMatrixStore";
        case EOpCooperativeMatrixMulAdd: return "EOpCooperativeMatrixMulAdd";
        case EOpCooperativeMatrixLoadNV: return "EOpCooperativeMatrixLoadNV";
        case EOpCooperativeMatrixStoreNV: return "EOpCooperativeMatrixStoreNV";
        case EOpCooperativeMatrixMulAddNV: return "EOpCooperativeMatrixMulAddNV";
        case EOpBeginInvocationInterlock: return "EOpBeginInvocationInterlock";
        case EOpEndInvocationInterlock: return "EOpEndInvocationInterlock";
        case EOpIsHelperInvocation: return "EOpIsHelperInvocation";
        case EOpDebugPrintf: return "EOpDebugPrintf";
        case EOpKill: return "EOpKill";
        case EOpTerminateInvocation: return "EOpTerminateInvocation";
        case EOpDemote: return "EOpDemote";
        case EOpTerminateRayKHR: return "EOpTerminateRayKHR";
        case EOpIgnoreIntersectionKHR: return "EOpIgnoreIntersectionKHR";
        case EOpReturn: return "EOpReturn";
        case EOpBreak: return "EOpBreak";
        case EOpContinue: return "EOpContinue";
        case EOpCase: return "EOpCase";
        case EOpDefault: return "EOpDefault";
        case EOpConstructGuardStart: return "EOpConstructGuardStart";
        case EOpConstructInt: return "EOpConstructInt";
        case EOpConstructUint: return "EOpConstructUint";
        case EOpConstructInt8: return "EOpConstructInt8";
        case EOpConstructUint8: return "EOpConstructUint8";
        case EOpConstructInt16: return "EOpConstructInt16";
        case EOpConstructUint16: return "EOpConstructUint16";
        case EOpConstructInt64: return "EOpConstructInt64";
        case EOpConstructUint64: return "EOpConstructUint64";
        case EOpConstructBool: return "EOpConstructBool";
        case EOpConstructFloat: return "EOpConstructFloat";
        case EOpConstructDouble: return "EOpConstructDouble";
        case EOpConstructVec2: return "EOpConstructVec2";
        case EOpConstructVec3: return "EOpConstructVec3";
        case EOpConstructVec4: return "EOpConstructVec4";
        case EOpConstructMat2x2: return "EOpConstructMat2x2";
        case EOpConstructMat2x3: return "EOpConstructMat2x3";
        case EOpConstructMat2x4: return "EOpConstructMat2x4";
        case EOpConstructMat3x2: return "EOpConstructMat3x2";
        case EOpConstructMat3x3: return "EOpConstructMat3x3";
        case EOpConstructMat3x4: return "EOpConstructMat3x4";
        case EOpConstructMat4x2: return "EOpConstructMat4x2";
        case EOpConstructMat4x3: return "EOpConstructMat4x3";
        case EOpConstructMat4x4: return "EOpConstructMat4x4";
        case EOpConstructDVec2: return "EOpConstructDVec2";
        case EOpConstructDVec3: return "EOpConstructDVec3";
        case EOpConstructDVec4: return "EOpConstructDVec4";
        case EOpConstructBVec2: return "EOpConstructBVec2";
        case EOpConstructBVec3: return "EOpConstructBVec3";
        case EOpConstructBVec4: return "EOpConstructBVec4";
        case EOpConstructI8Vec2: return "EOpConstructI8Vec2";
        case EOpConstructI8Vec3: return "EOpConstructI8Vec3";
        case EOpConstructI8Vec4: return "EOpConstructI8Vec4";
        case EOpConstructU8Vec2: return "EOpConstructU8Vec2";
        case EOpConstructU8Vec3: return "EOpConstructU8Vec3";
        case EOpConstructU8Vec4: return "EOpConstructU8Vec4";
        case EOpConstructI16Vec2: return "EOpConstructI16Vec2";
        case EOpConstructI16Vec3: return "EOpConstructI16Vec3";
        case EOpConstructI16Vec4: return "EOpConstructI16Vec4";
        case EOpConstructU16Vec2: return "EOpConstructU16Vec2";
        case EOpConstructU16Vec3: return "EOpConstructU16Vec3";
        case EOpConstructU16Vec4: return "EOpConstructU16Vec4";
        case EOpConstructIVec2: return "EOpConstructIVec2";
        case EOpConstructIVec3: return "EOpConstructIVec3";
        case EOpConstructIVec4: return "EOpConstructIVec4";
        case EOpConstructUVec2: return "EOpConstructUVec2";
        case EOpConstructUVec3: return "EOpConstructUVec3";
        case EOpConstructUVec4: return "EOpConstructUVec4";
        case EOpConstructI64Vec2: return "EOpConstructI64Vec2";
        case EOpConstructI64Vec3: return "EOpConstructI64Vec3";
        case EOpConstructI64Vec4: return "EOpConstructI64Vec4";
        case EOpConstructU64Vec2: return "EOpConstructU64Vec2";
        case EOpConstructU64Vec3: return "EOpConstructU64Vec3";
        case EOpConstructU64Vec4: return "EOpConstructU64Vec4";
        case EOpConstructDMat2x2: return "EOpConstructDMat2x2";
        case EOpConstructDMat2x3: return "EOpConstructDMat2x3";
        case EOpConstructDMat2x4: return "EOpConstructDMat2x4";
        case EOpConstructDMat3x2: return "EOpConstructDMat3x2";
        case EOpConstructDMat3x3: return "EOpConstructDMat3x3";
        case EOpConstructDMat3x4: return "EOpConstructDMat3x4";
        case EOpConstructDMat4x2: return "EOpConstructDMat4x2";
        case EOpConstructDMat4x3: return "EOpConstructDMat4x3";
        case EOpConstructDMat4x4: return "EOpConstructDMat4x4";
        case EOpConstructIMat2x2: return "EOpConstructIMat2x2";
        case EOpConstructIMat2x3: return "EOpConstructIMat2x3";
        case EOpConstructIMat2x4: return "EOpConstructIMat2x4";
        case EOpConstructIMat3x2: return "EOpConstructIMat3x2";
        case EOpConstructIMat3x3: return "EOpConstructIMat3x3";
        case EOpConstructIMat3x4: return "EOpConstructIMat3x4";
        case EOpConstructIMat4x2: return "EOpConstructIMat4x2";
        case EOpConstructIMat4x3: return "EOpConstructIMat4x3";
        case EOpConstructIMat4x4: return "EOpConstructIMat4x4";
        case EOpConstructUMat2x2: return "EOpConstructUMat2x2";
        case EOpConstructUMat2x3: return "EOpConstructUMat2x3";
        case EOpConstructUMat2x4: return "EOpConstructUMat2x4";
        case EOpConstructUMat3x2: return "EOpConstructUMat3x2";
        case EOpConstructUMat3x3: return "EOpConstructUMat3x3";
        case EOpConstructUMat3x4: return "EOpConstructUMat3x4";
        case EOpConstructUMat4x2: return "EOpConstructUMat4x2";
        case EOpConstructUMat4x3: return "EOpConstructUMat4x3";
        case EOpConstructUMat4x4: return "EOpConstructUMat4x4";
        case EOpConstructBMat2x2: return "EOpConstructBMat2x2";
        case EOpConstructBMat2x3: return "EOpConstructBMat2x3";
        case EOpConstructBMat2x4: return "EOpConstructBMat2x4";
        case EOpConstructBMat3x2: return "EOpConstructBMat3x2";
        case EOpConstructBMat3x3: return "EOpConstructBMat3x3";
        case EOpConstructBMat3x4: return "EOpConstructBMat3x4";
        case EOpConstructBMat4x2: return "EOpConstructBMat4x2";
        case EOpConstructBMat4x3: return "EOpConstructBMat4x3";
        case EOpConstructBMat4x4: return "EOpConstructBMat4x4";
        case EOpConstructFloat16: return "EOpConstructFloat16";
        case EOpConstructF16Vec2: return "EOpConstructF16Vec2";
        case EOpConstructF16Vec3: return "EOpConstructF16Vec3";
        case EOpConstructF16Vec4: return "EOpConstructF16Vec4";
        case EOpConstructF16Mat2x2: return "EOpConstructF16Mat2x2";
        case EOpConstructF16Mat2x3: return "EOpConstructF16Mat2x3";
        case EOpConstructF16Mat2x4: return "EOpConstructF16Mat2x4";
        case EOpConstructF16Mat3x2: return "EOpConstructF16Mat3x2";
        case EOpConstructF16Mat3x3: return "EOpConstructF16Mat3x3";
        case EOpConstructF16Mat3x4: return "EOpConstructF16Mat3x4";
        case EOpConstructF16Mat4x2: return "EOpConstructF16Mat4x2";
        case EOpConstructF16Mat4x3: return "EOpConstructF16Mat4x3";
        case EOpConstructF16Mat4x4: return "EOpConstructF16Mat4x4";
        case EOpConstructStruct: return "EOpConstructStruct";
        case EOpConstructTextureSampler: return "EOpConstructTextureSampler";
        case EOpConstructNonuniform: return "EOpConstructNonuniform";
        case EOpConstructReference: return "EOpConstructReference";
        case EOpConstructCooperativeMatrixNV: return "EOpConstructCooperativeMatrixNV";
        case EOpConstructCooperativeMatrixKHR: return "EOpConstructCooperativeMatrixKHR";
        case EOpConstructAccStruct: return "EOpConstructAccStruct";
        case EOpConstructGuardEnd: return "EOpConstructGuardEnd";
        case EOpAssign: return "EOpAssign";
        case EOpAddAssign: return "EOpAddAssign";
        case EOpSubAssign: return "EOpSubAssign";
        case EOpMulAssign: return "EOpMulAssign";
        case EOpVectorTimesMatrixAssign: return "EOpVectorTimesMatrixAssign";
        case EOpVectorTimesScalarAssign: return "EOpVectorTimesScalarAssign";
        case EOpMatrixTimesScalarAssign: return "EOpMatrixTimesScalarAssign";
        case EOpMatrixTimesMatrixAssign: return "EOpMatrixTimesMatrixAssign";
        case EOpDivAssign: return "EOpDivAssign";
        case EOpModAssign: return "EOpModAssign";
        case EOpAndAssign: return "EOpAndAssign";
        case EOpInclusiveOrAssign: return "EOpInclusiveOrAssign";
        case EOpExclusiveOrAssign: return "EOpExclusiveOrAssign";
        case EOpLeftShiftAssign: return "EOpLeftShiftAssign";
        case EOpRightShiftAssign: return "EOpRightShiftAssign";
        case EOpArrayLength: return "EOpArrayLength";
        case EOpImageGuardBegin: return "EOpImageGuardBegin";
        case EOpImageQuerySize: return "EOpImageQuerySize";
        case EOpImageQuerySamples: return "EOpImageQuerySamples";
        case EOpImageLoad: return "EOpImageLoad";
        case EOpImageStore: return "EOpImageStore";
        case EOpImageLoadLod: return "EOpImageLoadLod";
        case EOpImageStoreLod: return "EOpImageStoreLod";
        case EOpImageAtomicAdd: return "EOpImageAtomicAdd";
        case EOpImageAtomicMin: return "EOpImageAtomicMin";
        case EOpImageAtomicMax: return "EOpImageAtomicMax";
        case EOpImageAtomicAnd: return "EOpImageAtomicAnd";
        case EOpImageAtomicOr: return "EOpImageAtomicOr";
        case EOpImageAtomicXor: return "EOpImageAtomicXor";
        case EOpImageAtomicExchange: return "EOpImageAtomicExchange";
        case EOpImageAtomicCompSwap: return "EOpImageAtomicCompSwap";
        case EOpImageAtomicLoad: return "EOpImageAtomicLoad";
        case EOpImageAtomicStore: return "EOpImageAtomicStore";
        case EOpSubpassLoad: return "EOpSubpassLoad";
        case EOpSubpassLoadMS: return "EOpSubpassLoadMS";
        case EOpSparseImageLoad: return "EOpSparseImageLoad";
        case EOpSparseImageLoadLod: return "EOpSparseImageLoadLod";
        case EOpColorAttachmentReadEXT: return "EOpColorAttachmentReadEXT";
        case EOpImageGuardEnd: return "EOpImageGuardEnd";
        case EOpTextureGuardBegin: return "EOpTextureGuardBegin";
        case EOpTextureQuerySize: return "EOpTextureQuerySize";
        case EOpTextureQueryLod: return "EOpTextureQueryLod";
        case EOpTextureQueryLevels: return "EOpTextureQueryLevels";
        case EOpTextureQuerySamples: return "EOpTextureQuerySamples";
        case EOpSamplingGuardBegin: return "EOpSamplingGuardBegin";
        case EOpTexture: return "EOpTexture";
        case EOpTextureProj: return "EOpTextureProj";
        case EOpTextureLod: return "EOpTextureLod";
        case EOpTextureOffset: return "EOpTextureOffset";
        case EOpTextureFetch: return "EOpTextureFetch";
        case EOpTextureFetchOffset: return "EOpTextureFetchOffset";
        case EOpTextureProjOffset: return "EOpTextureProjOffset";
        case EOpTextureLodOffset: return "EOpTextureLodOffset";
        case EOpTextureProjLod: return "EOpTextureProjLod";
        case EOpTextureProjLodOffset: return "EOpTextureProjLodOffset";
        case EOpTextureGrad: return "EOpTextureGrad";
        case EOpTextureGradOffset: return "EOpTextureGradOffset";
        case EOpTextureProjGrad: return "EOpTextureProjGrad";
        case EOpTextureProjGradOffset: return "EOpTextureProjGradOffset";
        case EOpTextureGather: return "EOpTextureGather";
        case EOpTextureGatherOffset: return "EOpTextureGatherOffset";
        case EOpTextureGatherOffsets: return "EOpTextureGatherOffsets";
        case EOpTextureClamp: return "EOpTextureClamp";
        case EOpTextureOffsetClamp: return "EOpTextureOffsetClamp";
        case EOpTextureGradClamp: return "EOpTextureGradClamp";
        case EOpTextureGradOffsetClamp: return "EOpTextureGradOffsetClamp";
        case EOpTextureGatherLod: return "EOpTextureGatherLod";
        case EOpTextureGatherLodOffset: return "EOpTextureGatherLodOffset";
        case EOpTextureGatherLodOffsets: return "EOpTextureGatherLodOffsets";
        case EOpFragmentMaskFetch: return "EOpFragmentMaskFetch";
        case EOpFragmentFetch: return "EOpFragmentFetch";
        case EOpSparseTextureGuardBegin: return "EOpSparseTextureGuardBegin";
        case EOpSparseTexture: return "EOpSparseTexture";
        case EOpSparseTextureLod: return "EOpSparseTextureLod";
        case EOpSparseTextureOffset: return "EOpSparseTextureOffset";
        case EOpSparseTextureFetch: return "EOpSparseTextureFetch";
        case EOpSparseTextureFetchOffset: return "EOpSparseTextureFetchOffset";
        case EOpSparseTextureLodOffset: return "EOpSparseTextureLodOffset";
        case EOpSparseTextureGrad: return "EOpSparseTextureGrad";
        case EOpSparseTextureGradOffset: return "EOpSparseTextureGradOffset";
        case EOpSparseTextureGather: return "EOpSparseTextureGather";
        case EOpSparseTextureGatherOffset: return "EOpSparseTextureGatherOffset";
        case EOpSparseTextureGatherOffsets: return "EOpSparseTextureGatherOffsets";
        case EOpSparseTexelsResident: return "EOpSparseTexelsResident";
        case EOpSparseTextureClamp: return "EOpSparseTextureClamp";
        case EOpSparseTextureOffsetClamp: return "EOpSparseTextureOffsetClamp";
        case EOpSparseTextureGradClamp: return "EOpSparseTextureGradClamp";
        case EOpSparseTextureGradOffsetClamp: return "EOpSparseTextureGradOffsetClamp";
        case EOpSparseTextureGatherLod: return "EOpSparseTextureGatherLod";
        case EOpSparseTextureGatherLodOffset: return "EOpSparseTextureGatherLodOffset";
        case EOpSparseTextureGatherLodOffsets: return "EOpSparseTextureGatherLodOffsets";
        case EOpSparseTextureGuardEnd: return "EOpSparseTextureGuardEnd";
        case EOpImageFootprintGuardBegin: return "EOpImageFootprintGuardBegin";
        case EOpImageSampleFootprintNV: return "EOpImageSampleFootprintNV";
        case EOpImageSampleFootprintClampNV: return "EOpImageSampleFootprintClampNV";
        case EOpImageSampleFootprintLodNV: return "EOpImageSampleFootprintLodNV";
        case EOpImageSampleFootprintGradNV: return "EOpImageSampleFootprintGradNV";
        case EOpImageSampleFootprintGradClampNV: return "EOpImageSampleFootprintGradClampNV";
        case EOpImageFootprintGuardEnd: return "EOpImageFootprintGuardEnd";
        case EOpSamplingGuardEnd: return "EOpSamplingGuardEnd";
        case EOpTextureGuardEnd: return "EOpTextureGuardEnd";
        case EOpAddCarry: return "EOpAddCarry";
        case EOpSubBorrow: return "EOpSubBorrow";
        case EOpUMulExtended: return "EOpUMulExtended";
        case EOpIMulExtended: return "EOpIMulExtended";
        case EOpBitfieldExtract: return "EOpBitfieldExtract";
        case EOpBitfieldInsert: return "EOpBitfieldInsert";
        case EOpBitFieldReverse: return "EOpBitFieldReverse";
        case EOpBitCount: return "EOpBitCount";
        case EOpFindLSB: return "EOpFindLSB";
        case EOpFindMSB: return "EOpFindMSB";
        case EOpCountLeadingZeros: return "EOpCountLeadingZeros";
        case EOpCountTrailingZeros: return "EOpCountTrailingZeros";
        case EOpAbsDifference: return "EOpAbsDifference";
        case EOpAddSaturate: return "EOpAddSaturate";
        case EOpSubSaturate: return "EOpSubSaturate";
        case EOpAverage: return "EOpAverage";
        case EOpAverageRounded: return "EOpAverageRounded";
        case EOpMul32x16: return "EOpMul32x16";
        case EOpTraceNV: return "EOpTraceNV";
        case EOpTraceRayMotionNV: return "EOpTraceRayMotionNV";
        case EOpTraceKHR: return "EOpTraceKHR";
        case EOpReportIntersection: return "EOpReportIntersection";
        case EOpIgnoreIntersectionNV: return "EOpIgnoreIntersectionNV";
        case EOpTerminateRayNV: return "EOpTerminateRayNV";
        case EOpExecuteCallableNV: return "EOpExecuteCallableNV";
        case EOpExecuteCallableKHR: return "EOpExecuteCallableKHR";
        case EOpWritePackedPrimitiveIndices4x8NV: return "EOpWritePackedPrimitiveIndices4x8NV";
        case EOpEmitMeshTasksEXT: return "EOpEmitMeshTasksEXT";
        case EOpSetMeshOutputsEXT: return "EOpSetMeshOutputsEXT";
        case EOpRayQueryInitialize: return "EOpRayQueryInitialize";
        case EOpRayQueryTerminate: return "EOpRayQueryTerminate";
        case EOpRayQueryGenerateIntersection: return "EOpRayQueryGenerateIntersection";
        case EOpRayQueryConfirmIntersection: return "EOpRayQueryConfirmIntersection";
        case EOpRayQueryProceed: return "EOpRayQueryProceed";
        case EOpRayQueryGetIntersectionType: return "EOpRayQueryGetIntersectionType";
        case EOpRayQueryGetRayTMin: return "EOpRayQueryGetRayTMin";
        case EOpRayQueryGetRayFlags: return "EOpRayQueryGetRayFlags";
        case EOpRayQueryGetIntersectionT: return "EOpRayQueryGetIntersectionT";
        case EOpRayQueryGetIntersectionInstanceCustomIndex: return "EOpRayQueryGetIntersectionInstanceCustomIndex";
        case EOpRayQueryGetIntersectionInstanceId: return "EOpRayQueryGetIntersectionInstanceId";
        case EOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffset: return "EOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffset";
        case EOpRayQueryGetIntersectionGeometryIndex: return "EOpRayQueryGetIntersectionGeometryIndex";
        case EOpRayQueryGetIntersectionPrimitiveIndex: return "EOpRayQueryGetIntersectionPrimitiveIndex";
        case EOpRayQueryGetIntersectionBarycentrics: return "EOpRayQueryGetIntersectionBarycentrics";
        case EOpRayQueryGetIntersectionFrontFace: return "EOpRayQueryGetIntersectionFrontFace";
        case EOpRayQueryGetIntersectionCandidateAABBOpaque: return "EOpRayQueryGetIntersectionCandidateAABBOpaque";
        case EOpRayQueryGetIntersectionObjectRayDirection: return "EOpRayQueryGetIntersectionObjectRayDirection";
        case EOpRayQueryGetIntersectionObjectRayOrigin: return "EOpRayQueryGetIntersectionObjectRayOrigin";
        case EOpRayQueryGetWorldRayDirection: return "EOpRayQueryGetWorldRayDirection";
        case EOpRayQueryGetWorldRayOrigin: return "EOpRayQueryGetWorldRayOrigin";
        case EOpRayQueryGetIntersectionObjectToWorld: return "EOpRayQueryGetIntersectionObjectToWorld";
        case EOpRayQueryGetIntersectionWorldToObject: return "EOpRayQueryGetIntersectionWorldToObject";
        case EOpHitObjectTraceRayNV: return "EOpHitObjectTraceRayNV";
        case EOpHitObjectTraceRayMotionNV: return "EOpHitObjectTraceRayMotionNV";
        case EOpHitObjectRecordHitNV: return "EOpHitObjectRecordHitNV";
        case EOpHitObjectRecordHitMotionNV: return "EOpHitObjectRecordHitMotionNV";
        case EOpHitObjectRecordHitWithIndexNV: return "EOpHitObjectRecordHitWithIndexNV";
        case EOpHitObjectRecordHitWithIndexMotionNV: return "EOpHitObjectRecordHitWithIndexMotionNV";
        case EOpHitObjectRecordMissNV: return "EOpHitObjectRecordMissNV";
        case EOpHitObjectRecordMissMotionNV: return "EOpHitObjectRecordMissMotionNV";
        case EOpHitObjectRecordEmptyNV: return "EOpHitObjectRecordEmptyNV";
        case EOpHitObjectExecuteShaderNV: return "EOpHitObjectExecuteShaderNV";
        case EOpHitObjectIsEmptyNV: return "EOpHitObjectIsEmptyNV";
        case EOpHitObjectIsMissNV: return "EOpHitObjectIsMissNV";
        case EOpHitObjectIsHitNV: return "EOpHitObjectIsHitNV";
        case EOpHitObjectGetRayTMinNV: return "EOpHitObjectGetRayTMinNV";
        case EOpHitObjectGetRayTMaxNV: return "EOpHitObjectGetRayTMaxNV";
        case EOpHitObjectGetObjectRayOriginNV: return "EOpHitObjectGetObjectRayOriginNV";
        case EOpHitObjectGetObjectRayDirectionNV: return "EOpHitObjectGetObjectRayDirectionNV";
        case EOpHitObjectGetWorldRayOriginNV: return "EOpHitObjectGetWorldRayOriginNV";
        case EOpHitObjectGetWorldRayDirectionNV: return "EOpHitObjectGetWorldRayDirectionNV";
        case EOpHitObjectGetWorldToObjectNV: return "EOpHitObjectGetWorldToObjectNV";
        case EOpHitObjectGetObjectToWorldNV: return "EOpHitObjectGetObjectToWorldNV";
        case EOpHitObjectGetInstanceCustomIndexNV: return "EOpHitObjectGetInstanceCustomIndexNV";
        case EOpHitObjectGetInstanceIdNV: return "EOpHitObjectGetInstanceIdNV";
        case EOpHitObjectGetGeometryIndexNV: return "EOpHitObjectGetGeometryIndexNV";
        case EOpHitObjectGetPrimitiveIndexNV: return "EOpHitObjectGetPrimitiveIndexNV";
        case EOpHitObjectGetHitKindNV: return "EOpHitObjectGetHitKindNV";
        case EOpHitObjectGetShaderBindingTableRecordIndexNV: return "EOpHitObjectGetShaderBindingTableRecordIndexNV";
        case EOpHitObjectGetShaderRecordBufferHandleNV: return "EOpHitObjectGetShaderRecordBufferHandleNV";
        case EOpHitObjectGetAttributesNV: return "EOpHitObjectGetAttributesNV";
        case EOpHitObjectGetCurrentTimeNV: return "EOpHitObjectGetCurrentTimeNV";
        case EOpReorderThreadNV: return "EOpReorderThreadNV";
        case EOpFetchMicroTriangleVertexPositionNV: return "EOpFetchMicroTriangleVertexPositionNV";
        case EOpFetchMicroTriangleVertexBarycentricNV: return "EOpFetchMicroTriangleVertexBarycentricNV";
        case EOpClip: return "EOpClip";
        case EOpIsFinite: return "EOpIsFinite";
        case EOpLog10: return "EOpLog10";
        case EOpRcp: return "EOpRcp";
        case EOpSaturate: return "EOpSaturate";
        case EOpSinCos: return "EOpSinCos";
        case EOpGenMul: return "EOpGenMul";
        case EOpDst: return "EOpDst";
        case EOpInterlockedAdd: return "EOpInterlockedAdd";
        case EOpInterlockedAnd: return "EOpInterlockedAnd";
        case EOpInterlockedCompareExchange: return "EOpInterlockedCompareExchange";
        case EOpInterlockedCompareStore: return "EOpInterlockedCompareStore";
        case EOpInterlockedExchange: return "EOpInterlockedExchange";
        case EOpInterlockedMax: return "EOpInterlockedMax";
        case EOpInterlockedMin: return "EOpInterlockedMin";
        case EOpInterlockedOr: return "EOpInterlockedOr";
        case EOpInterlockedXor: return "EOpInterlockedXor";
        case EOpAllMemoryBarrierWithGroupSync: return "EOpAllMemoryBarrierWithGroupSync";
        case EOpDeviceMemoryBarrier: return "EOpDeviceMemoryBarrier";
        case EOpDeviceMemoryBarrierWithGroupSync: return "EOpDeviceMemoryBarrierWithGroupSync";
        case EOpWorkgroupMemoryBarrier: return "EOpWorkgroupMemoryBarrier";
        case EOpWorkgroupMemoryBarrierWithGroupSync: return "EOpWorkgroupMemoryBarrierWithGroupSync";
        case EOpEvaluateAttributeSnapped: return "EOpEvaluateAttributeSnapped";
        case EOpF32tof16: return "EOpF32tof16";
        case EOpF16tof32: return "EOpF16tof32";
        case EOpLit: return "EOpLit";
        case EOpTextureBias: return "EOpTextureBias";
        case EOpAsDouble: return "EOpAsDouble";
        case EOpD3DCOLORtoUBYTE4: return "EOpD3DCOLORtoUBYTE4";
        case EOpMethodSample: return "EOpMethodSample";
        case EOpMethodSampleBias: return "EOpMethodSampleBias";
        case EOpMethodSampleCmp: return "EOpMethodSampleCmp";
        case EOpMethodSampleCmpLevelZero: return "EOpMethodSampleCmpLevelZero";
        case EOpMethodSampleGrad: return "EOpMethodSampleGrad";
        case EOpMethodSampleLevel: return "EOpMethodSampleLevel";
        case EOpMethodLoad: return "EOpMethodLoad";
        case EOpMethodGetDimensions: return "EOpMethodGetDimensions";
        case EOpMethodGetSamplePosition: return "EOpMethodGetSamplePosition";
        case EOpMethodGather: return "EOpMethodGather";
        case EOpMethodCalculateLevelOfDetail: return "EOpMethodCalculateLevelOfDetail";
        case EOpMethodCalculateLevelOfDetailUnclamped: return "EOpMethodCalculateLevelOfDetailUnclamped";
        case EOpMethodLoad2: return "EOpMethodLoad2";
        case EOpMethodLoad3: return "EOpMethodLoad3";
        case EOpMethodLoad4: return "EOpMethodLoad4";
        case EOpMethodStore: return "EOpMethodStore";
        case EOpMethodStore2: return "EOpMethodStore2";
        case EOpMethodStore3: return "EOpMethodStore3";
        case EOpMethodStore4: return "EOpMethodStore4";
        case EOpMethodIncrementCounter: return "EOpMethodIncrementCounter";
        case EOpMethodDecrementCounter: return "EOpMethodDecrementCounter";
        case EOpMethodConsume: return "EOpMethodConsume";
        case EOpMethodGatherRed: return "EOpMethodGatherRed";
        case EOpMethodGatherGreen: return "EOpMethodGatherGreen";
        case EOpMethodGatherBlue: return "EOpMethodGatherBlue";
        case EOpMethodGatherAlpha: return "EOpMethodGatherAlpha";
        case EOpMethodGatherCmp: return "EOpMethodGatherCmp";
        case EOpMethodGatherCmpRed: return "EOpMethodGatherCmpRed";
        case EOpMethodGatherCmpGreen: return "EOpMethodGatherCmpGreen";
        case EOpMethodGatherCmpBlue: return "EOpMethodGatherCmpBlue";
        case EOpMethodGatherCmpAlpha: return "EOpMethodGatherCmpAlpha";
        case EOpMethodAppend: return "EOpMethodAppend";
        case EOpMethodRestartStrip: return "EOpMethodRestartStrip";
        case EOpMatrixSwizzle: return "EOpMatrixSwizzle";
        case EOpWaveGetLaneCount: return "EOpWaveGetLaneCount";
        case EOpWaveGetLaneIndex: return "EOpWaveGetLaneIndex";
        case EOpWaveActiveCountBits: return "EOpWaveActiveCountBits";
        case EOpWavePrefixCountBits: return "EOpWavePrefixCountBits";
        case EOpReadClockSubgroupKHR: return "EOpReadClockSubgroupKHR";
        case EOpReadClockDeviceKHR: return "EOpReadClockDeviceKHR";
        case EOpRayQueryGetIntersectionTriangleVertexPositionsEXT: return "EOpRayQueryGetIntersectionTriangleVertexPositionsEXT";
        case EOpStencilAttachmentReadEXT: return "EOpStencilAttachmentReadEXT";
        case EOpDepthAttachmentReadEXT: return "EOpDepthAttachmentReadEXT";
        case EOpImageSampleWeightedQCOM: return "EOpImageSampleWeightedQCOM";
        case EOpImageBoxFilterQCOM: return "EOpImageBoxFilterQCOM";
        case EOpImageBlockMatchSADQCOM: return "EOpImageBlockMatchSADQCOM";
        case EOpImageBlockMatchSSDQCOM: return "EOpImageBlockMatchSSDQCOM";
        default: return std::to_string((int)op);
    }
}

std::string nodeToString(const TIntermNode* node) {
    if (auto nodeAsOperator = node->getAsOperator()) {
        std::string result;
        if (node->getAsAggregate()) {
            result = "Aggregate(";
        } else if (node->getAsUnaryNode()) {
            result = "Unary(";
        } else if (node->getAsBinaryNode()) {
            result = "Binary(";
        } else {
            result = "Operator(";
        }
        result += operatorToString(nodeAsOperator->getOp());
        result += ')';
        return result;
    }
    if (node->getAsLoopNode()) {
        return "Loop";
    }
    if (node->getAsBranchNode()) {
        return "Branch";
    }
    if (node->getAsSymbolNode()) {
        return "Symbol";
    }
    if (node->getAsMethodNode()) {
        return "Method";
    }
    if (node->getAsSwitchNode()) {
        return "Switch";
    }
    if (node->getAsSelectionNode()) {
        return "Selection";
    }
    if (auto nodeAsConstantUnion = node->getAsConstantUnion()) {
        return "ConstantUnion(size = "
                + std::to_string(nodeAsConstantUnion->getConstArray().size())
                + ")";
    }
    if (node->getAsTyped()) {
        return "Typed";
    }
    return "Node";
}

std::string locToString(const glslang::TSourceLoc& loc) {
    return loc.getStringNameOrNum() + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
}

RValueOperator glslangOperatorToRValueOperator(glslang::TOperator op) {
    switch (op) {
        case EOpNegative: return RValueOperator::Negative;
        case EOpLogicalNot: return RValueOperator::LogicalNot;
        case EOpVectorLogicalNot: return RValueOperator::VectorLogicalNot;
        case EOpBitwiseNot: return RValueOperator::BitwiseNot;
        case EOpPostIncrement: return RValueOperator::PostIncrement;
        case EOpPostDecrement: return RValueOperator::PostDecrement;
        case EOpPreIncrement: return RValueOperator::PreIncrement;
        case EOpPreDecrement: return RValueOperator::PreDecrement;
        case EOpCopyObject: return RValueOperator::CopyObject;
        case EOpDeclare: return RValueOperator::Declare;
        case EOpConvInt8ToBool: return RValueOperator::ConvInt8ToBool;
        case EOpConvUint8ToBool: return RValueOperator::ConvUint8ToBool;
        case EOpConvInt16ToBool: return RValueOperator::ConvInt16ToBool;
        case EOpConvUint16ToBool: return RValueOperator::ConvUint16ToBool;
        case EOpConvIntToBool: return RValueOperator::ConvIntToBool;
        case EOpConvUintToBool: return RValueOperator::ConvUintToBool;
        case EOpConvInt64ToBool: return RValueOperator::ConvInt64ToBool;
        case EOpConvUint64ToBool: return RValueOperator::ConvUint64ToBool;
        case EOpConvFloat16ToBool: return RValueOperator::ConvFloat16ToBool;
        case EOpConvFloatToBool: return RValueOperator::ConvFloatToBool;
        case EOpConvDoubleToBool: return RValueOperator::ConvDoubleToBool;
        case EOpConvBoolToInt8: return RValueOperator::ConvBoolToInt8;
        case EOpConvBoolToUint8: return RValueOperator::ConvBoolToUint8;
        case EOpConvBoolToInt16: return RValueOperator::ConvBoolToInt16;
        case EOpConvBoolToUint16: return RValueOperator::ConvBoolToUint16;
        case EOpConvBoolToInt: return RValueOperator::ConvBoolToInt;
        case EOpConvBoolToUint: return RValueOperator::ConvBoolToUint;
        case EOpConvBoolToInt64: return RValueOperator::ConvBoolToInt64;
        case EOpConvBoolToUint64: return RValueOperator::ConvBoolToUint64;
        case EOpConvBoolToFloat16: return RValueOperator::ConvBoolToFloat16;
        case EOpConvBoolToFloat: return RValueOperator::ConvBoolToFloat;
        case EOpConvBoolToDouble: return RValueOperator::ConvBoolToDouble;
        case EOpConvInt8ToInt16: return RValueOperator::ConvInt8ToInt16;
        case EOpConvInt8ToInt: return RValueOperator::ConvInt8ToInt;
        case EOpConvInt8ToInt64: return RValueOperator::ConvInt8ToInt64;
        case EOpConvInt8ToUint8: return RValueOperator::ConvInt8ToUint8;
        case EOpConvInt8ToUint16: return RValueOperator::ConvInt8ToUint16;
        case EOpConvInt8ToUint: return RValueOperator::ConvInt8ToUint;
        case EOpConvInt8ToUint64: return RValueOperator::ConvInt8ToUint64;
        case EOpConvUint8ToInt8: return RValueOperator::ConvUint8ToInt8;
        case EOpConvUint8ToInt16: return RValueOperator::ConvUint8ToInt16;
        case EOpConvUint8ToInt: return RValueOperator::ConvUint8ToInt;
        case EOpConvUint8ToInt64: return RValueOperator::ConvUint8ToInt64;
        case EOpConvUint8ToUint16: return RValueOperator::ConvUint8ToUint16;
        case EOpConvUint8ToUint: return RValueOperator::ConvUint8ToUint;
        case EOpConvUint8ToUint64: return RValueOperator::ConvUint8ToUint64;
        case EOpConvInt8ToFloat16: return RValueOperator::ConvInt8ToFloat16;
        case EOpConvInt8ToFloat: return RValueOperator::ConvInt8ToFloat;
        case EOpConvInt8ToDouble: return RValueOperator::ConvInt8ToDouble;
        case EOpConvUint8ToFloat16: return RValueOperator::ConvUint8ToFloat16;
        case EOpConvUint8ToFloat: return RValueOperator::ConvUint8ToFloat;
        case EOpConvUint8ToDouble: return RValueOperator::ConvUint8ToDouble;
        case EOpConvInt16ToInt8: return RValueOperator::ConvInt16ToInt8;
        case EOpConvInt16ToInt: return RValueOperator::ConvInt16ToInt;
        case EOpConvInt16ToInt64: return RValueOperator::ConvInt16ToInt64;
        case EOpConvInt16ToUint8: return RValueOperator::ConvInt16ToUint8;
        case EOpConvInt16ToUint16: return RValueOperator::ConvInt16ToUint16;
        case EOpConvInt16ToUint: return RValueOperator::ConvInt16ToUint;
        case EOpConvInt16ToUint64: return RValueOperator::ConvInt16ToUint64;
        case EOpConvUint16ToInt8: return RValueOperator::ConvUint16ToInt8;
        case EOpConvUint16ToInt16: return RValueOperator::ConvUint16ToInt16;
        case EOpConvUint16ToInt: return RValueOperator::ConvUint16ToInt;
        case EOpConvUint16ToInt64: return RValueOperator::ConvUint16ToInt64;
        case EOpConvUint16ToUint8: return RValueOperator::ConvUint16ToUint8;
        case EOpConvUint16ToUint: return RValueOperator::ConvUint16ToUint;
        case EOpConvUint16ToUint64: return RValueOperator::ConvUint16ToUint64;
        case EOpConvInt16ToFloat16: return RValueOperator::ConvInt16ToFloat16;
        case EOpConvInt16ToFloat: return RValueOperator::ConvInt16ToFloat;
        case EOpConvInt16ToDouble: return RValueOperator::ConvInt16ToDouble;
        case EOpConvUint16ToFloat16: return RValueOperator::ConvUint16ToFloat16;
        case EOpConvUint16ToFloat: return RValueOperator::ConvUint16ToFloat;
        case EOpConvUint16ToDouble: return RValueOperator::ConvUint16ToDouble;
        case EOpConvIntToInt8: return RValueOperator::ConvIntToInt8;
        case EOpConvIntToInt16: return RValueOperator::ConvIntToInt16;
        case EOpConvIntToInt64: return RValueOperator::ConvIntToInt64;
        case EOpConvIntToUint8: return RValueOperator::ConvIntToUint8;
        case EOpConvIntToUint16: return RValueOperator::ConvIntToUint16;
        case EOpConvIntToUint: return RValueOperator::ConvIntToUint;
        case EOpConvIntToUint64: return RValueOperator::ConvIntToUint64;
        case EOpConvUintToInt8: return RValueOperator::ConvUintToInt8;
        case EOpConvUintToInt16: return RValueOperator::ConvUintToInt16;
        case EOpConvUintToInt: return RValueOperator::ConvUintToInt;
        case EOpConvUintToInt64: return RValueOperator::ConvUintToInt64;
        case EOpConvUintToUint8: return RValueOperator::ConvUintToUint8;
        case EOpConvUintToUint16: return RValueOperator::ConvUintToUint16;
        case EOpConvUintToUint64: return RValueOperator::ConvUintToUint64;
        case EOpConvIntToFloat16: return RValueOperator::ConvIntToFloat16;
        case EOpConvIntToFloat: return RValueOperator::ConvIntToFloat;
        case EOpConvIntToDouble: return RValueOperator::ConvIntToDouble;
        case EOpConvUintToFloat16: return RValueOperator::ConvUintToFloat16;
        case EOpConvUintToFloat: return RValueOperator::ConvUintToFloat;
        case EOpConvUintToDouble: return RValueOperator::ConvUintToDouble;
        case EOpConvInt64ToInt8: return RValueOperator::ConvInt64ToInt8;
        case EOpConvInt64ToInt16: return RValueOperator::ConvInt64ToInt16;
        case EOpConvInt64ToInt: return RValueOperator::ConvInt64ToInt;
        case EOpConvInt64ToUint8: return RValueOperator::ConvInt64ToUint8;
        case EOpConvInt64ToUint16: return RValueOperator::ConvInt64ToUint16;
        case EOpConvInt64ToUint: return RValueOperator::ConvInt64ToUint;
        case EOpConvInt64ToUint64: return RValueOperator::ConvInt64ToUint64;
        case EOpConvUint64ToInt8: return RValueOperator::ConvUint64ToInt8;
        case EOpConvUint64ToInt16: return RValueOperator::ConvUint64ToInt16;
        case EOpConvUint64ToInt: return RValueOperator::ConvUint64ToInt;
        case EOpConvUint64ToInt64: return RValueOperator::ConvUint64ToInt64;
        case EOpConvUint64ToUint8: return RValueOperator::ConvUint64ToUint8;
        case EOpConvUint64ToUint16: return RValueOperator::ConvUint64ToUint16;
        case EOpConvUint64ToUint: return RValueOperator::ConvUint64ToUint;
        case EOpConvInt64ToFloat16: return RValueOperator::ConvInt64ToFloat16;
        case EOpConvInt64ToFloat: return RValueOperator::ConvInt64ToFloat;
        case EOpConvInt64ToDouble: return RValueOperator::ConvInt64ToDouble;
        case EOpConvUint64ToFloat16: return RValueOperator::ConvUint64ToFloat16;
        case EOpConvUint64ToFloat: return RValueOperator::ConvUint64ToFloat;
        case EOpConvUint64ToDouble: return RValueOperator::ConvUint64ToDouble;
        case EOpConvFloat16ToInt8: return RValueOperator::ConvFloat16ToInt8;
        case EOpConvFloat16ToInt16: return RValueOperator::ConvFloat16ToInt16;
        case EOpConvFloat16ToInt: return RValueOperator::ConvFloat16ToInt;
        case EOpConvFloat16ToInt64: return RValueOperator::ConvFloat16ToInt64;
        case EOpConvFloat16ToUint8: return RValueOperator::ConvFloat16ToUint8;
        case EOpConvFloat16ToUint16: return RValueOperator::ConvFloat16ToUint16;
        case EOpConvFloat16ToUint: return RValueOperator::ConvFloat16ToUint;
        case EOpConvFloat16ToUint64: return RValueOperator::ConvFloat16ToUint64;
        case EOpConvFloat16ToFloat: return RValueOperator::ConvFloat16ToFloat;
        case EOpConvFloat16ToDouble: return RValueOperator::ConvFloat16ToDouble;
        case EOpConvFloatToInt8: return RValueOperator::ConvFloatToInt8;
        case EOpConvFloatToInt16: return RValueOperator::ConvFloatToInt16;
        case EOpConvFloatToInt: return RValueOperator::ConvFloatToInt;
        case EOpConvFloatToInt64: return RValueOperator::ConvFloatToInt64;
        case EOpConvFloatToUint8: return RValueOperator::ConvFloatToUint8;
        case EOpConvFloatToUint16: return RValueOperator::ConvFloatToUint16;
        case EOpConvFloatToUint: return RValueOperator::ConvFloatToUint;
        case EOpConvFloatToUint64: return RValueOperator::ConvFloatToUint64;
        case EOpConvFloatToFloat16: return RValueOperator::ConvFloatToFloat16;
        case EOpConvFloatToDouble: return RValueOperator::ConvFloatToDouble;
        case EOpConvDoubleToInt8: return RValueOperator::ConvDoubleToInt8;
        case EOpConvDoubleToInt16: return RValueOperator::ConvDoubleToInt16;
        case EOpConvDoubleToInt: return RValueOperator::ConvDoubleToInt;
        case EOpConvDoubleToInt64: return RValueOperator::ConvDoubleToInt64;
        case EOpConvDoubleToUint8: return RValueOperator::ConvDoubleToUint8;
        case EOpConvDoubleToUint16: return RValueOperator::ConvDoubleToUint16;
        case EOpConvDoubleToUint: return RValueOperator::ConvDoubleToUint;
        case EOpConvDoubleToUint64: return RValueOperator::ConvDoubleToUint64;
        case EOpConvDoubleToFloat16: return RValueOperator::ConvDoubleToFloat16;
        case EOpConvDoubleToFloat: return RValueOperator::ConvDoubleToFloat;
        case EOpConvUint64ToPtr: return RValueOperator::ConvUint64ToPtr;
        case EOpConvPtrToUint64: return RValueOperator::ConvPtrToUint64;
        case EOpConvUvec2ToPtr: return RValueOperator::ConvUvec2ToPtr;
        case EOpConvPtrToUvec2: return RValueOperator::ConvPtrToUvec2;
        case EOpConvUint64ToAccStruct: return RValueOperator::ConvUint64ToAccStruct;
        case EOpConvUvec2ToAccStruct: return RValueOperator::ConvUvec2ToAccStruct;
        case EOpAdd: return RValueOperator::Add;
        case EOpSub: return RValueOperator::Sub;
        case EOpMul: return RValueOperator::Mul;
        case EOpDiv: return RValueOperator::Div;
        case EOpMod: return RValueOperator::Mod;
        case EOpRightShift: return RValueOperator::RightShift;
        case EOpLeftShift: return RValueOperator::LeftShift;
        case EOpAnd: return RValueOperator::And;
        case EOpInclusiveOr: return RValueOperator::InclusiveOr;
        case EOpExclusiveOr: return RValueOperator::ExclusiveOr;
        case EOpEqual: return RValueOperator::Equal;
        case EOpNotEqual: return RValueOperator::NotEqual;
        case EOpVectorEqual: return RValueOperator::VectorEqual;
        case EOpVectorNotEqual: return RValueOperator::VectorNotEqual;
        case EOpLessThan: return RValueOperator::LessThan;
        case EOpGreaterThan: return RValueOperator::GreaterThan;
        case EOpLessThanEqual: return RValueOperator::LessThanEqual;
        case EOpGreaterThanEqual: return RValueOperator::GreaterThanEqual;
        case EOpComma: return RValueOperator::Comma;
        case EOpVectorTimesScalar: return RValueOperator::VectorTimesScalar;
        case EOpVectorTimesMatrix: return RValueOperator::VectorTimesMatrix;
        case EOpMatrixTimesVector: return RValueOperator::MatrixTimesVector;
        case EOpMatrixTimesScalar: return RValueOperator::MatrixTimesScalar;
        case EOpLogicalOr: return RValueOperator::LogicalOr;
        case EOpLogicalXor: return RValueOperator::LogicalXor;
        case EOpLogicalAnd: return RValueOperator::LogicalAnd;
        case EOpIndexDirect: return RValueOperator::IndexDirect;
        case EOpIndexIndirect: return RValueOperator::IndexIndirect;
        case EOpIndexDirectStruct: return RValueOperator::IndexDirectStruct;
        case EOpVectorSwizzle: return RValueOperator::VectorSwizzle;
        case EOpMethod: return RValueOperator::Method;
        case EOpScoping: return RValueOperator::Scoping;
        case EOpRadians: return RValueOperator::Radians;
        case EOpDegrees: return RValueOperator::Degrees;
        case EOpSin: return RValueOperator::Sin;
        case EOpCos: return RValueOperator::Cos;
        case EOpTan: return RValueOperator::Tan;
        case EOpAsin: return RValueOperator::Asin;
        case EOpAcos: return RValueOperator::Acos;
        case EOpAtan: return RValueOperator::Atan;
        case EOpSinh: return RValueOperator::Sinh;
        case EOpCosh: return RValueOperator::Cosh;
        case EOpTanh: return RValueOperator::Tanh;
        case EOpAsinh: return RValueOperator::Asinh;
        case EOpAcosh: return RValueOperator::Acosh;
        case EOpAtanh: return RValueOperator::Atanh;
        case EOpPow: return RValueOperator::Pow;
        case EOpExp: return RValueOperator::Exp;
        case EOpLog: return RValueOperator::Log;
        case EOpExp2: return RValueOperator::Exp2;
        case EOpLog2: return RValueOperator::Log2;
        case EOpSqrt: return RValueOperator::Sqrt;
        case EOpInverseSqrt: return RValueOperator::InverseSqrt;
        case EOpAbs: return RValueOperator::Abs;
        case EOpSign: return RValueOperator::Sign;
        case EOpFloor: return RValueOperator::Floor;
        case EOpTrunc: return RValueOperator::Trunc;
        case EOpRound: return RValueOperator::Round;
        case EOpRoundEven: return RValueOperator::RoundEven;
        case EOpCeil: return RValueOperator::Ceil;
        case EOpFract: return RValueOperator::Fract;
        case EOpModf: return RValueOperator::Modf;
        case EOpMin: return RValueOperator::Min;
        case EOpMax: return RValueOperator::Max;
        case EOpClamp: return RValueOperator::Clamp;
        case EOpMix: return RValueOperator::Mix;
        case EOpStep: return RValueOperator::Step;
        case EOpSmoothStep: return RValueOperator::SmoothStep;
        case EOpIsNan: return RValueOperator::IsNan;
        case EOpIsInf: return RValueOperator::IsInf;
        case EOpFma: return RValueOperator::Fma;
        case EOpFrexp: return RValueOperator::Frexp;
        case EOpLdexp: return RValueOperator::Ldexp;
        case EOpFloatBitsToInt: return RValueOperator::FloatBitsToInt;
        case EOpFloatBitsToUint: return RValueOperator::FloatBitsToUint;
        case EOpIntBitsToFloat: return RValueOperator::IntBitsToFloat;
        case EOpUintBitsToFloat: return RValueOperator::UintBitsToFloat;
        case EOpDoubleBitsToInt64: return RValueOperator::DoubleBitsToInt64;
        case EOpDoubleBitsToUint64: return RValueOperator::DoubleBitsToUint64;
        case EOpInt64BitsToDouble: return RValueOperator::Int64BitsToDouble;
        case EOpUint64BitsToDouble: return RValueOperator::Uint64BitsToDouble;
        case EOpFloat16BitsToInt16: return RValueOperator::Float16BitsToInt16;
        case EOpFloat16BitsToUint16: return RValueOperator::Float16BitsToUint16;
        case EOpInt16BitsToFloat16: return RValueOperator::Int16BitsToFloat16;
        case EOpUint16BitsToFloat16: return RValueOperator::Uint16BitsToFloat16;
        case EOpPackSnorm2x16: return RValueOperator::PackSnorm2x16;
        case EOpUnpackSnorm2x16: return RValueOperator::UnpackSnorm2x16;
        case EOpPackUnorm2x16: return RValueOperator::PackUnorm2x16;
        case EOpUnpackUnorm2x16: return RValueOperator::UnpackUnorm2x16;
        case EOpPackSnorm4x8: return RValueOperator::PackSnorm4x8;
        case EOpUnpackSnorm4x8: return RValueOperator::UnpackSnorm4x8;
        case EOpPackUnorm4x8: return RValueOperator::PackUnorm4x8;
        case EOpUnpackUnorm4x8: return RValueOperator::UnpackUnorm4x8;
        case EOpPackHalf2x16: return RValueOperator::PackHalf2x16;
        case EOpUnpackHalf2x16: return RValueOperator::UnpackHalf2x16;
        case EOpPackDouble2x32: return RValueOperator::PackDouble2x32;
        case EOpUnpackDouble2x32: return RValueOperator::UnpackDouble2x32;
        case EOpPackInt2x32: return RValueOperator::PackInt2x32;
        case EOpUnpackInt2x32: return RValueOperator::UnpackInt2x32;
        case EOpPackUint2x32: return RValueOperator::PackUint2x32;
        case EOpUnpackUint2x32: return RValueOperator::UnpackUint2x32;
        case EOpPackFloat2x16: return RValueOperator::PackFloat2x16;
        case EOpUnpackFloat2x16: return RValueOperator::UnpackFloat2x16;
        case EOpPackInt2x16: return RValueOperator::PackInt2x16;
        case EOpUnpackInt2x16: return RValueOperator::UnpackInt2x16;
        case EOpPackUint2x16: return RValueOperator::PackUint2x16;
        case EOpUnpackUint2x16: return RValueOperator::UnpackUint2x16;
        case EOpPackInt4x16: return RValueOperator::PackInt4x16;
        case EOpUnpackInt4x16: return RValueOperator::UnpackInt4x16;
        case EOpPackUint4x16: return RValueOperator::PackUint4x16;
        case EOpUnpackUint4x16: return RValueOperator::UnpackUint4x16;
        case EOpPack16: return RValueOperator::Pack16;
        case EOpPack32: return RValueOperator::Pack32;
        case EOpPack64: return RValueOperator::Pack64;
        case EOpUnpack32: return RValueOperator::Unpack32;
        case EOpUnpack16: return RValueOperator::Unpack16;
        case EOpUnpack8: return RValueOperator::Unpack8;
        case EOpLength: return RValueOperator::Length;
        case EOpDistance: return RValueOperator::Distance;
        case EOpDot: return RValueOperator::Dot;
        case EOpCross: return RValueOperator::Cross;
        case EOpNormalize: return RValueOperator::Normalize;
        case EOpFaceForward: return RValueOperator::FaceForward;
        case EOpReflect: return RValueOperator::Reflect;
        case EOpRefract: return RValueOperator::Refract;
        case EOpMin3: return RValueOperator::Min3;
        case EOpMax3: return RValueOperator::Max3;
        case EOpMid3: return RValueOperator::Mid3;
        case EOpDPdx: return RValueOperator::DPdx;
        case EOpDPdy: return RValueOperator::DPdy;
        case EOpFwidth: return RValueOperator::Fwidth;
        case EOpDPdxFine: return RValueOperator::DPdxFine;
        case EOpDPdyFine: return RValueOperator::DPdyFine;
        case EOpFwidthFine: return RValueOperator::FwidthFine;
        case EOpDPdxCoarse: return RValueOperator::DPdxCoarse;
        case EOpDPdyCoarse: return RValueOperator::DPdyCoarse;
        case EOpFwidthCoarse: return RValueOperator::FwidthCoarse;
        case EOpInterpolateAtCentroid: return RValueOperator::InterpolateAtCentroid;
        case EOpInterpolateAtSample: return RValueOperator::InterpolateAtSample;
        case EOpInterpolateAtOffset: return RValueOperator::InterpolateAtOffset;
        case EOpInterpolateAtVertex: return RValueOperator::InterpolateAtVertex;
        case EOpMatrixTimesMatrix: return RValueOperator::MatrixTimesMatrix;
        case EOpOuterProduct: return RValueOperator::OuterProduct;
        case EOpDeterminant: return RValueOperator::Determinant;
        case EOpMatrixInverse: return RValueOperator::MatrixInverse;
        case EOpTranspose: return RValueOperator::Transpose;
        case EOpFtransform: return RValueOperator::Ftransform;
        case EOpNoise: return RValueOperator::Noise;
        case EOpEmitVertex: return RValueOperator::EmitVertex;
        case EOpEndPrimitive: return RValueOperator::EndPrimitive;
        case EOpEmitStreamVertex: return RValueOperator::EmitStreamVertex;
        case EOpEndStreamPrimitive: return RValueOperator::EndStreamPrimitive;
        case EOpBarrier: return RValueOperator::Barrier;
        case EOpMemoryBarrier: return RValueOperator::MemoryBarrier;
        case EOpMemoryBarrierAtomicCounter: return RValueOperator::MemoryBarrierAtomicCounter;
        case EOpMemoryBarrierBuffer: return RValueOperator::MemoryBarrierBuffer;
        case EOpMemoryBarrierImage: return RValueOperator::MemoryBarrierImage;
        case EOpMemoryBarrierShared: return RValueOperator::MemoryBarrierShared;
        case EOpGroupMemoryBarrier: return RValueOperator::GroupMemoryBarrier;
        case EOpBallot: return RValueOperator::Ballot;
        case EOpReadInvocation: return RValueOperator::ReadInvocation;
        case EOpReadFirstInvocation: return RValueOperator::ReadFirstInvocation;
        case EOpAnyInvocation: return RValueOperator::AnyInvocation;
        case EOpAllInvocations: return RValueOperator::AllInvocations;
        case EOpAllInvocationsEqual: return RValueOperator::AllInvocationsEqual;
        case EOpSubgroupGuardStart: return RValueOperator::SubgroupGuardStart;
        case EOpSubgroupBarrier: return RValueOperator::SubgroupBarrier;
        case EOpSubgroupMemoryBarrier: return RValueOperator::SubgroupMemoryBarrier;
        case EOpSubgroupMemoryBarrierBuffer: return RValueOperator::SubgroupMemoryBarrierBuffer;
        case EOpSubgroupMemoryBarrierImage: return RValueOperator::SubgroupMemoryBarrierImage;
        case EOpSubgroupMemoryBarrierShared: return RValueOperator::SubgroupMemoryBarrierShared;
        case EOpSubgroupElect: return RValueOperator::SubgroupElect;
        case EOpSubgroupAll: return RValueOperator::SubgroupAll;
        case EOpSubgroupAny: return RValueOperator::SubgroupAny;
        case EOpSubgroupAllEqual: return RValueOperator::SubgroupAllEqual;
        case EOpSubgroupBroadcast: return RValueOperator::SubgroupBroadcast;
        case EOpSubgroupBroadcastFirst: return RValueOperator::SubgroupBroadcastFirst;
        case EOpSubgroupBallot: return RValueOperator::SubgroupBallot;
        case EOpSubgroupInverseBallot: return RValueOperator::SubgroupInverseBallot;
        case EOpSubgroupBallotBitExtract: return RValueOperator::SubgroupBallotBitExtract;
        case EOpSubgroupBallotBitCount: return RValueOperator::SubgroupBallotBitCount;
        case EOpSubgroupBallotInclusiveBitCount: return RValueOperator::SubgroupBallotInclusiveBitCount;
        case EOpSubgroupBallotExclusiveBitCount: return RValueOperator::SubgroupBallotExclusiveBitCount;
        case EOpSubgroupBallotFindLSB: return RValueOperator::SubgroupBallotFindLSB;
        case EOpSubgroupBallotFindMSB: return RValueOperator::SubgroupBallotFindMSB;
        case EOpSubgroupShuffle: return RValueOperator::SubgroupShuffle;
        case EOpSubgroupShuffleXor: return RValueOperator::SubgroupShuffleXor;
        case EOpSubgroupShuffleUp: return RValueOperator::SubgroupShuffleUp;
        case EOpSubgroupShuffleDown: return RValueOperator::SubgroupShuffleDown;
        case EOpSubgroupAdd: return RValueOperator::SubgroupAdd;
        case EOpSubgroupMul: return RValueOperator::SubgroupMul;
        case EOpSubgroupMin: return RValueOperator::SubgroupMin;
        case EOpSubgroupMax: return RValueOperator::SubgroupMax;
        case EOpSubgroupAnd: return RValueOperator::SubgroupAnd;
        case EOpSubgroupOr: return RValueOperator::SubgroupOr;
        case EOpSubgroupXor: return RValueOperator::SubgroupXor;
        case EOpSubgroupInclusiveAdd: return RValueOperator::SubgroupInclusiveAdd;
        case EOpSubgroupInclusiveMul: return RValueOperator::SubgroupInclusiveMul;
        case EOpSubgroupInclusiveMin: return RValueOperator::SubgroupInclusiveMin;
        case EOpSubgroupInclusiveMax: return RValueOperator::SubgroupInclusiveMax;
        case EOpSubgroupInclusiveAnd: return RValueOperator::SubgroupInclusiveAnd;
        case EOpSubgroupInclusiveOr: return RValueOperator::SubgroupInclusiveOr;
        case EOpSubgroupInclusiveXor: return RValueOperator::SubgroupInclusiveXor;
        case EOpSubgroupExclusiveAdd: return RValueOperator::SubgroupExclusiveAdd;
        case EOpSubgroupExclusiveMul: return RValueOperator::SubgroupExclusiveMul;
        case EOpSubgroupExclusiveMin: return RValueOperator::SubgroupExclusiveMin;
        case EOpSubgroupExclusiveMax: return RValueOperator::SubgroupExclusiveMax;
        case EOpSubgroupExclusiveAnd: return RValueOperator::SubgroupExclusiveAnd;
        case EOpSubgroupExclusiveOr: return RValueOperator::SubgroupExclusiveOr;
        case EOpSubgroupExclusiveXor: return RValueOperator::SubgroupExclusiveXor;
        case EOpSubgroupClusteredAdd: return RValueOperator::SubgroupClusteredAdd;
        case EOpSubgroupClusteredMul: return RValueOperator::SubgroupClusteredMul;
        case EOpSubgroupClusteredMin: return RValueOperator::SubgroupClusteredMin;
        case EOpSubgroupClusteredMax: return RValueOperator::SubgroupClusteredMax;
        case EOpSubgroupClusteredAnd: return RValueOperator::SubgroupClusteredAnd;
        case EOpSubgroupClusteredOr: return RValueOperator::SubgroupClusteredOr;
        case EOpSubgroupClusteredXor: return RValueOperator::SubgroupClusteredXor;
        case EOpSubgroupQuadBroadcast: return RValueOperator::SubgroupQuadBroadcast;
        case EOpSubgroupQuadSwapHorizontal: return RValueOperator::SubgroupQuadSwapHorizontal;
        case EOpSubgroupQuadSwapVertical: return RValueOperator::SubgroupQuadSwapVertical;
        case EOpSubgroupQuadSwapDiagonal: return RValueOperator::SubgroupQuadSwapDiagonal;
        case EOpSubgroupPartition: return RValueOperator::SubgroupPartition;
        case EOpSubgroupPartitionedAdd: return RValueOperator::SubgroupPartitionedAdd;
        case EOpSubgroupPartitionedMul: return RValueOperator::SubgroupPartitionedMul;
        case EOpSubgroupPartitionedMin: return RValueOperator::SubgroupPartitionedMin;
        case EOpSubgroupPartitionedMax: return RValueOperator::SubgroupPartitionedMax;
        case EOpSubgroupPartitionedAnd: return RValueOperator::SubgroupPartitionedAnd;
        case EOpSubgroupPartitionedOr: return RValueOperator::SubgroupPartitionedOr;
        case EOpSubgroupPartitionedXor: return RValueOperator::SubgroupPartitionedXor;
        case EOpSubgroupPartitionedInclusiveAdd: return RValueOperator::SubgroupPartitionedInclusiveAdd;
        case EOpSubgroupPartitionedInclusiveMul: return RValueOperator::SubgroupPartitionedInclusiveMul;
        case EOpSubgroupPartitionedInclusiveMin: return RValueOperator::SubgroupPartitionedInclusiveMin;
        case EOpSubgroupPartitionedInclusiveMax: return RValueOperator::SubgroupPartitionedInclusiveMax;
        case EOpSubgroupPartitionedInclusiveAnd: return RValueOperator::SubgroupPartitionedInclusiveAnd;
        case EOpSubgroupPartitionedInclusiveOr: return RValueOperator::SubgroupPartitionedInclusiveOr;
        case EOpSubgroupPartitionedInclusiveXor: return RValueOperator::SubgroupPartitionedInclusiveXor;
        case EOpSubgroupPartitionedExclusiveAdd: return RValueOperator::SubgroupPartitionedExclusiveAdd;
        case EOpSubgroupPartitionedExclusiveMul: return RValueOperator::SubgroupPartitionedExclusiveMul;
        case EOpSubgroupPartitionedExclusiveMin: return RValueOperator::SubgroupPartitionedExclusiveMin;
        case EOpSubgroupPartitionedExclusiveMax: return RValueOperator::SubgroupPartitionedExclusiveMax;
        case EOpSubgroupPartitionedExclusiveAnd: return RValueOperator::SubgroupPartitionedExclusiveAnd;
        case EOpSubgroupPartitionedExclusiveOr: return RValueOperator::SubgroupPartitionedExclusiveOr;
        case EOpSubgroupPartitionedExclusiveXor: return RValueOperator::SubgroupPartitionedExclusiveXor;
        case EOpSubgroupGuardStop: return RValueOperator::SubgroupGuardStop;
        case EOpMinInvocations: return RValueOperator::MinInvocations;
        case EOpMaxInvocations: return RValueOperator::MaxInvocations;
        case EOpAddInvocations: return RValueOperator::AddInvocations;
        case EOpMinInvocationsNonUniform: return RValueOperator::MinInvocationsNonUniform;
        case EOpMaxInvocationsNonUniform: return RValueOperator::MaxInvocationsNonUniform;
        case EOpAddInvocationsNonUniform: return RValueOperator::AddInvocationsNonUniform;
        case EOpMinInvocationsInclusiveScan: return RValueOperator::MinInvocationsInclusiveScan;
        case EOpMaxInvocationsInclusiveScan: return RValueOperator::MaxInvocationsInclusiveScan;
        case EOpAddInvocationsInclusiveScan: return RValueOperator::AddInvocationsInclusiveScan;
        case EOpMinInvocationsInclusiveScanNonUniform: return RValueOperator::MinInvocationsInclusiveScanNonUniform;
        case EOpMaxInvocationsInclusiveScanNonUniform: return RValueOperator::MaxInvocationsInclusiveScanNonUniform;
        case EOpAddInvocationsInclusiveScanNonUniform: return RValueOperator::AddInvocationsInclusiveScanNonUniform;
        case EOpMinInvocationsExclusiveScan: return RValueOperator::MinInvocationsExclusiveScan;
        case EOpMaxInvocationsExclusiveScan: return RValueOperator::MaxInvocationsExclusiveScan;
        case EOpAddInvocationsExclusiveScan: return RValueOperator::AddInvocationsExclusiveScan;
        case EOpMinInvocationsExclusiveScanNonUniform: return RValueOperator::MinInvocationsExclusiveScanNonUniform;
        case EOpMaxInvocationsExclusiveScanNonUniform: return RValueOperator::MaxInvocationsExclusiveScanNonUniform;
        case EOpAddInvocationsExclusiveScanNonUniform: return RValueOperator::AddInvocationsExclusiveScanNonUniform;
        case EOpSwizzleInvocations: return RValueOperator::SwizzleInvocations;
        case EOpSwizzleInvocationsMasked: return RValueOperator::SwizzleInvocationsMasked;
        case EOpWriteInvocation: return RValueOperator::WriteInvocation;
        case EOpMbcnt: return RValueOperator::Mbcnt;
        case EOpCubeFaceIndex: return RValueOperator::CubeFaceIndex;
        case EOpCubeFaceCoord: return RValueOperator::CubeFaceCoord;
        case EOpTime: return RValueOperator::Time;
        case EOpAtomicAdd: return RValueOperator::AtomicAdd;
        case EOpAtomicSubtract: return RValueOperator::AtomicSubtract;
        case EOpAtomicMin: return RValueOperator::AtomicMin;
        case EOpAtomicMax: return RValueOperator::AtomicMax;
        case EOpAtomicAnd: return RValueOperator::AtomicAnd;
        case EOpAtomicOr: return RValueOperator::AtomicOr;
        case EOpAtomicXor: return RValueOperator::AtomicXor;
        case EOpAtomicExchange: return RValueOperator::AtomicExchange;
        case EOpAtomicCompSwap: return RValueOperator::AtomicCompSwap;
        case EOpAtomicLoad: return RValueOperator::AtomicLoad;
        case EOpAtomicStore: return RValueOperator::AtomicStore;
        case EOpAtomicCounterIncrement: return RValueOperator::AtomicCounterIncrement;
        case EOpAtomicCounterDecrement: return RValueOperator::AtomicCounterDecrement;
        case EOpAtomicCounter: return RValueOperator::AtomicCounter;
        case EOpAtomicCounterAdd: return RValueOperator::AtomicCounterAdd;
        case EOpAtomicCounterSubtract: return RValueOperator::AtomicCounterSubtract;
        case EOpAtomicCounterMin: return RValueOperator::AtomicCounterMin;
        case EOpAtomicCounterMax: return RValueOperator::AtomicCounterMax;
        case EOpAtomicCounterAnd: return RValueOperator::AtomicCounterAnd;
        case EOpAtomicCounterOr: return RValueOperator::AtomicCounterOr;
        case EOpAtomicCounterXor: return RValueOperator::AtomicCounterXor;
        case EOpAtomicCounterExchange: return RValueOperator::AtomicCounterExchange;
        case EOpAtomicCounterCompSwap: return RValueOperator::AtomicCounterCompSwap;
        case EOpAny: return RValueOperator::Any;
        case EOpAll: return RValueOperator::All;
        case EOpCooperativeMatrixLoad: return RValueOperator::CooperativeMatrixLoad;
        case EOpCooperativeMatrixStore: return RValueOperator::CooperativeMatrixStore;
        case EOpCooperativeMatrixMulAdd: return RValueOperator::CooperativeMatrixMulAdd;
        case EOpCooperativeMatrixLoadNV: return RValueOperator::CooperativeMatrixLoadNV;
        case EOpCooperativeMatrixStoreNV: return RValueOperator::CooperativeMatrixStoreNV;
        case EOpCooperativeMatrixMulAddNV: return RValueOperator::CooperativeMatrixMulAddNV;
        case EOpBeginInvocationInterlock: return RValueOperator::BeginInvocationInterlock;
        case EOpEndInvocationInterlock: return RValueOperator::EndInvocationInterlock;
        case EOpIsHelperInvocation: return RValueOperator::IsHelperInvocation;
        case EOpDebugPrintf: return RValueOperator::DebugPrintf;
        case EOpConstructGuardStart: return RValueOperator::ConstructGuardStart;
        case EOpConstructInt: return RValueOperator::ConstructInt;
        case EOpConstructUint: return RValueOperator::ConstructUint;
        case EOpConstructInt8: return RValueOperator::ConstructInt8;
        case EOpConstructUint8: return RValueOperator::ConstructUint8;
        case EOpConstructInt16: return RValueOperator::ConstructInt16;
        case EOpConstructUint16: return RValueOperator::ConstructUint16;
        case EOpConstructInt64: return RValueOperator::ConstructInt64;
        case EOpConstructUint64: return RValueOperator::ConstructUint64;
        case EOpConstructBool: return RValueOperator::ConstructBool;
        case EOpConstructFloat: return RValueOperator::ConstructFloat;
        case EOpConstructDouble: return RValueOperator::ConstructDouble;
        case EOpConstructVec2: return RValueOperator::ConstructVec2;
        case EOpConstructVec3: return RValueOperator::ConstructVec3;
        case EOpConstructVec4: return RValueOperator::ConstructVec4;
        case EOpConstructMat2x2: return RValueOperator::ConstructMat2x2;
        case EOpConstructMat2x3: return RValueOperator::ConstructMat2x3;
        case EOpConstructMat2x4: return RValueOperator::ConstructMat2x4;
        case EOpConstructMat3x2: return RValueOperator::ConstructMat3x2;
        case EOpConstructMat3x3: return RValueOperator::ConstructMat3x3;
        case EOpConstructMat3x4: return RValueOperator::ConstructMat3x4;
        case EOpConstructMat4x2: return RValueOperator::ConstructMat4x2;
        case EOpConstructMat4x3: return RValueOperator::ConstructMat4x3;
        case EOpConstructMat4x4: return RValueOperator::ConstructMat4x4;
        case EOpConstructDVec2: return RValueOperator::ConstructDVec2;
        case EOpConstructDVec3: return RValueOperator::ConstructDVec3;
        case EOpConstructDVec4: return RValueOperator::ConstructDVec4;
        case EOpConstructBVec2: return RValueOperator::ConstructBVec2;
        case EOpConstructBVec3: return RValueOperator::ConstructBVec3;
        case EOpConstructBVec4: return RValueOperator::ConstructBVec4;
        case EOpConstructI8Vec2: return RValueOperator::ConstructI8Vec2;
        case EOpConstructI8Vec3: return RValueOperator::ConstructI8Vec3;
        case EOpConstructI8Vec4: return RValueOperator::ConstructI8Vec4;
        case EOpConstructU8Vec2: return RValueOperator::ConstructU8Vec2;
        case EOpConstructU8Vec3: return RValueOperator::ConstructU8Vec3;
        case EOpConstructU8Vec4: return RValueOperator::ConstructU8Vec4;
        case EOpConstructI16Vec2: return RValueOperator::ConstructI16Vec2;
        case EOpConstructI16Vec3: return RValueOperator::ConstructI16Vec3;
        case EOpConstructI16Vec4: return RValueOperator::ConstructI16Vec4;
        case EOpConstructU16Vec2: return RValueOperator::ConstructU16Vec2;
        case EOpConstructU16Vec3: return RValueOperator::ConstructU16Vec3;
        case EOpConstructU16Vec4: return RValueOperator::ConstructU16Vec4;
        case EOpConstructIVec2: return RValueOperator::ConstructIVec2;
        case EOpConstructIVec3: return RValueOperator::ConstructIVec3;
        case EOpConstructIVec4: return RValueOperator::ConstructIVec4;
        case EOpConstructUVec2: return RValueOperator::ConstructUVec2;
        case EOpConstructUVec3: return RValueOperator::ConstructUVec3;
        case EOpConstructUVec4: return RValueOperator::ConstructUVec4;
        case EOpConstructI64Vec2: return RValueOperator::ConstructI64Vec2;
        case EOpConstructI64Vec3: return RValueOperator::ConstructI64Vec3;
        case EOpConstructI64Vec4: return RValueOperator::ConstructI64Vec4;
        case EOpConstructU64Vec2: return RValueOperator::ConstructU64Vec2;
        case EOpConstructU64Vec3: return RValueOperator::ConstructU64Vec3;
        case EOpConstructU64Vec4: return RValueOperator::ConstructU64Vec4;
        case EOpConstructDMat2x2: return RValueOperator::ConstructDMat2x2;
        case EOpConstructDMat2x3: return RValueOperator::ConstructDMat2x3;
        case EOpConstructDMat2x4: return RValueOperator::ConstructDMat2x4;
        case EOpConstructDMat3x2: return RValueOperator::ConstructDMat3x2;
        case EOpConstructDMat3x3: return RValueOperator::ConstructDMat3x3;
        case EOpConstructDMat3x4: return RValueOperator::ConstructDMat3x4;
        case EOpConstructDMat4x2: return RValueOperator::ConstructDMat4x2;
        case EOpConstructDMat4x3: return RValueOperator::ConstructDMat4x3;
        case EOpConstructDMat4x4: return RValueOperator::ConstructDMat4x4;
        case EOpConstructIMat2x2: return RValueOperator::ConstructIMat2x2;
        case EOpConstructIMat2x3: return RValueOperator::ConstructIMat2x3;
        case EOpConstructIMat2x4: return RValueOperator::ConstructIMat2x4;
        case EOpConstructIMat3x2: return RValueOperator::ConstructIMat3x2;
        case EOpConstructIMat3x3: return RValueOperator::ConstructIMat3x3;
        case EOpConstructIMat3x4: return RValueOperator::ConstructIMat3x4;
        case EOpConstructIMat4x2: return RValueOperator::ConstructIMat4x2;
        case EOpConstructIMat4x3: return RValueOperator::ConstructIMat4x3;
        case EOpConstructIMat4x4: return RValueOperator::ConstructIMat4x4;
        case EOpConstructUMat2x2: return RValueOperator::ConstructUMat2x2;
        case EOpConstructUMat2x3: return RValueOperator::ConstructUMat2x3;
        case EOpConstructUMat2x4: return RValueOperator::ConstructUMat2x4;
        case EOpConstructUMat3x2: return RValueOperator::ConstructUMat3x2;
        case EOpConstructUMat3x3: return RValueOperator::ConstructUMat3x3;
        case EOpConstructUMat3x4: return RValueOperator::ConstructUMat3x4;
        case EOpConstructUMat4x2: return RValueOperator::ConstructUMat4x2;
        case EOpConstructUMat4x3: return RValueOperator::ConstructUMat4x3;
        case EOpConstructUMat4x4: return RValueOperator::ConstructUMat4x4;
        case EOpConstructBMat2x2: return RValueOperator::ConstructBMat2x2;
        case EOpConstructBMat2x3: return RValueOperator::ConstructBMat2x3;
        case EOpConstructBMat2x4: return RValueOperator::ConstructBMat2x4;
        case EOpConstructBMat3x2: return RValueOperator::ConstructBMat3x2;
        case EOpConstructBMat3x3: return RValueOperator::ConstructBMat3x3;
        case EOpConstructBMat3x4: return RValueOperator::ConstructBMat3x4;
        case EOpConstructBMat4x2: return RValueOperator::ConstructBMat4x2;
        case EOpConstructBMat4x3: return RValueOperator::ConstructBMat4x3;
        case EOpConstructBMat4x4: return RValueOperator::ConstructBMat4x4;
        case EOpConstructFloat16: return RValueOperator::ConstructFloat16;
        case EOpConstructF16Vec2: return RValueOperator::ConstructF16Vec2;
        case EOpConstructF16Vec3: return RValueOperator::ConstructF16Vec3;
        case EOpConstructF16Vec4: return RValueOperator::ConstructF16Vec4;
        case EOpConstructF16Mat2x2: return RValueOperator::ConstructF16Mat2x2;
        case EOpConstructF16Mat2x3: return RValueOperator::ConstructF16Mat2x3;
        case EOpConstructF16Mat2x4: return RValueOperator::ConstructF16Mat2x4;
        case EOpConstructF16Mat3x2: return RValueOperator::ConstructF16Mat3x2;
        case EOpConstructF16Mat3x3: return RValueOperator::ConstructF16Mat3x3;
        case EOpConstructF16Mat3x4: return RValueOperator::ConstructF16Mat3x4;
        case EOpConstructF16Mat4x2: return RValueOperator::ConstructF16Mat4x2;
        case EOpConstructF16Mat4x3: return RValueOperator::ConstructF16Mat4x3;
        case EOpConstructF16Mat4x4: return RValueOperator::ConstructF16Mat4x4;
        case EOpConstructStruct: return RValueOperator::ConstructStruct;
        case EOpConstructTextureSampler: return RValueOperator::ConstructTextureSampler;
        case EOpConstructNonuniform: return RValueOperator::ConstructNonuniform;
        case EOpConstructReference: return RValueOperator::ConstructReference;
        case EOpConstructCooperativeMatrixNV: return RValueOperator::ConstructCooperativeMatrixNV;
        case EOpConstructCooperativeMatrixKHR: return RValueOperator::ConstructCooperativeMatrixKHR;
        case EOpConstructAccStruct: return RValueOperator::ConstructAccStruct;
        case EOpConstructGuardEnd: return RValueOperator::ConstructGuardEnd;
        case EOpAssign: return RValueOperator::Assign;
        case EOpAddAssign: return RValueOperator::AddAssign;
        case EOpSubAssign: return RValueOperator::SubAssign;
        case EOpMulAssign: return RValueOperator::MulAssign;
        case EOpVectorTimesMatrixAssign: return RValueOperator::VectorTimesMatrixAssign;
        case EOpVectorTimesScalarAssign: return RValueOperator::VectorTimesScalarAssign;
        case EOpMatrixTimesScalarAssign: return RValueOperator::MatrixTimesScalarAssign;
        case EOpMatrixTimesMatrixAssign: return RValueOperator::MatrixTimesMatrixAssign;
        case EOpDivAssign: return RValueOperator::DivAssign;
        case EOpModAssign: return RValueOperator::ModAssign;
        case EOpAndAssign: return RValueOperator::AndAssign;
        case EOpInclusiveOrAssign: return RValueOperator::InclusiveOrAssign;
        case EOpExclusiveOrAssign: return RValueOperator::ExclusiveOrAssign;
        case EOpLeftShiftAssign: return RValueOperator::LeftShiftAssign;
        case EOpRightShiftAssign: return RValueOperator::RightShiftAssign;
        case EOpArrayLength: return RValueOperator::ArrayLength;
        case EOpImageGuardBegin: return RValueOperator::ImageGuardBegin;
        case EOpImageQuerySize: return RValueOperator::ImageQuerySize;
        case EOpImageQuerySamples: return RValueOperator::ImageQuerySamples;
        case EOpImageLoad: return RValueOperator::ImageLoad;
        case EOpImageStore: return RValueOperator::ImageStore;
        case EOpImageLoadLod: return RValueOperator::ImageLoadLod;
        case EOpImageStoreLod: return RValueOperator::ImageStoreLod;
        case EOpImageAtomicAdd: return RValueOperator::ImageAtomicAdd;
        case EOpImageAtomicMin: return RValueOperator::ImageAtomicMin;
        case EOpImageAtomicMax: return RValueOperator::ImageAtomicMax;
        case EOpImageAtomicAnd: return RValueOperator::ImageAtomicAnd;
        case EOpImageAtomicOr: return RValueOperator::ImageAtomicOr;
        case EOpImageAtomicXor: return RValueOperator::ImageAtomicXor;
        case EOpImageAtomicExchange: return RValueOperator::ImageAtomicExchange;
        case EOpImageAtomicCompSwap: return RValueOperator::ImageAtomicCompSwap;
        case EOpImageAtomicLoad: return RValueOperator::ImageAtomicLoad;
        case EOpImageAtomicStore: return RValueOperator::ImageAtomicStore;
        case EOpSubpassLoad: return RValueOperator::SubpassLoad;
        case EOpSubpassLoadMS: return RValueOperator::SubpassLoadMS;
        case EOpSparseImageLoad: return RValueOperator::SparseImageLoad;
        case EOpSparseImageLoadLod: return RValueOperator::SparseImageLoadLod;
        case EOpColorAttachmentReadEXT: return RValueOperator::ColorAttachmentReadEXT;
        case EOpImageGuardEnd: return RValueOperator::ImageGuardEnd;
        case EOpTextureGuardBegin: return RValueOperator::TextureGuardBegin;
        case EOpTextureQuerySize: return RValueOperator::TextureQuerySize;
        case EOpTextureQueryLod: return RValueOperator::TextureQueryLod;
        case EOpTextureQueryLevels: return RValueOperator::TextureQueryLevels;
        case EOpTextureQuerySamples: return RValueOperator::TextureQuerySamples;
        case EOpSamplingGuardBegin: return RValueOperator::SamplingGuardBegin;
        case EOpTexture: return RValueOperator::Texture;
        case EOpTextureProj: return RValueOperator::TextureProj;
        case EOpTextureLod: return RValueOperator::TextureLod;
        case EOpTextureOffset: return RValueOperator::TextureOffset;
        case EOpTextureFetch: return RValueOperator::TextureFetch;
        case EOpTextureFetchOffset: return RValueOperator::TextureFetchOffset;
        case EOpTextureProjOffset: return RValueOperator::TextureProjOffset;
        case EOpTextureLodOffset: return RValueOperator::TextureLodOffset;
        case EOpTextureProjLod: return RValueOperator::TextureProjLod;
        case EOpTextureProjLodOffset: return RValueOperator::TextureProjLodOffset;
        case EOpTextureGrad: return RValueOperator::TextureGrad;
        case EOpTextureGradOffset: return RValueOperator::TextureGradOffset;
        case EOpTextureProjGrad: return RValueOperator::TextureProjGrad;
        case EOpTextureProjGradOffset: return RValueOperator::TextureProjGradOffset;
        case EOpTextureGather: return RValueOperator::TextureGather;
        case EOpTextureGatherOffset: return RValueOperator::TextureGatherOffset;
        case EOpTextureGatherOffsets: return RValueOperator::TextureGatherOffsets;
        case EOpTextureClamp: return RValueOperator::TextureClamp;
        case EOpTextureOffsetClamp: return RValueOperator::TextureOffsetClamp;
        case EOpTextureGradClamp: return RValueOperator::TextureGradClamp;
        case EOpTextureGradOffsetClamp: return RValueOperator::TextureGradOffsetClamp;
        case EOpTextureGatherLod: return RValueOperator::TextureGatherLod;
        case EOpTextureGatherLodOffset: return RValueOperator::TextureGatherLodOffset;
        case EOpTextureGatherLodOffsets: return RValueOperator::TextureGatherLodOffsets;
        case EOpFragmentMaskFetch: return RValueOperator::FragmentMaskFetch;
        case EOpFragmentFetch: return RValueOperator::FragmentFetch;
        case EOpSparseTextureGuardBegin: return RValueOperator::SparseTextureGuardBegin;
        case EOpSparseTexture: return RValueOperator::SparseTexture;
        case EOpSparseTextureLod: return RValueOperator::SparseTextureLod;
        case EOpSparseTextureOffset: return RValueOperator::SparseTextureOffset;
        case EOpSparseTextureFetch: return RValueOperator::SparseTextureFetch;
        case EOpSparseTextureFetchOffset: return RValueOperator::SparseTextureFetchOffset;
        case EOpSparseTextureLodOffset: return RValueOperator::SparseTextureLodOffset;
        case EOpSparseTextureGrad: return RValueOperator::SparseTextureGrad;
        case EOpSparseTextureGradOffset: return RValueOperator::SparseTextureGradOffset;
        case EOpSparseTextureGather: return RValueOperator::SparseTextureGather;
        case EOpSparseTextureGatherOffset: return RValueOperator::SparseTextureGatherOffset;
        case EOpSparseTextureGatherOffsets: return RValueOperator::SparseTextureGatherOffsets;
        case EOpSparseTexelsResident: return RValueOperator::SparseTexelsResident;
        case EOpSparseTextureClamp: return RValueOperator::SparseTextureClamp;
        case EOpSparseTextureOffsetClamp: return RValueOperator::SparseTextureOffsetClamp;
        case EOpSparseTextureGradClamp: return RValueOperator::SparseTextureGradClamp;
        case EOpSparseTextureGradOffsetClamp: return RValueOperator::SparseTextureGradOffsetClamp;
        case EOpSparseTextureGatherLod: return RValueOperator::SparseTextureGatherLod;
        case EOpSparseTextureGatherLodOffset: return RValueOperator::SparseTextureGatherLodOffset;
        case EOpSparseTextureGatherLodOffsets: return RValueOperator::SparseTextureGatherLodOffsets;
        case EOpSparseTextureGuardEnd: return RValueOperator::SparseTextureGuardEnd;
        case EOpImageFootprintGuardBegin: return RValueOperator::ImageFootprintGuardBegin;
        case EOpImageSampleFootprintNV: return RValueOperator::ImageSampleFootprintNV;
        case EOpImageSampleFootprintClampNV: return RValueOperator::ImageSampleFootprintClampNV;
        case EOpImageSampleFootprintLodNV: return RValueOperator::ImageSampleFootprintLodNV;
        case EOpImageSampleFootprintGradNV: return RValueOperator::ImageSampleFootprintGradNV;
        case EOpImageSampleFootprintGradClampNV: return RValueOperator::ImageSampleFootprintGradClampNV;
        case EOpImageFootprintGuardEnd: return RValueOperator::ImageFootprintGuardEnd;
        case EOpSamplingGuardEnd: return RValueOperator::SamplingGuardEnd;
        case EOpTextureGuardEnd: return RValueOperator::TextureGuardEnd;
        case EOpAddCarry: return RValueOperator::AddCarry;
        case EOpSubBorrow: return RValueOperator::SubBorrow;
        case EOpUMulExtended: return RValueOperator::UMulExtended;
        case EOpIMulExtended: return RValueOperator::IMulExtended;
        case EOpBitfieldExtract: return RValueOperator::BitfieldExtract;
        case EOpBitfieldInsert: return RValueOperator::BitfieldInsert;
        case EOpBitFieldReverse: return RValueOperator::BitFieldReverse;
        case EOpBitCount: return RValueOperator::BitCount;
        case EOpFindLSB: return RValueOperator::FindLSB;
        case EOpFindMSB: return RValueOperator::FindMSB;
        case EOpCountLeadingZeros: return RValueOperator::CountLeadingZeros;
        case EOpCountTrailingZeros: return RValueOperator::CountTrailingZeros;
        case EOpAbsDifference: return RValueOperator::AbsDifference;
        case EOpAddSaturate: return RValueOperator::AddSaturate;
        case EOpSubSaturate: return RValueOperator::SubSaturate;
        case EOpAverage: return RValueOperator::Average;
        case EOpAverageRounded: return RValueOperator::AverageRounded;
        case EOpMul32x16: return RValueOperator::Mul32x16;
        case EOpTraceNV: return RValueOperator::TraceNV;
        case EOpTraceRayMotionNV: return RValueOperator::TraceRayMotionNV;
        case EOpTraceKHR: return RValueOperator::TraceKHR;
        case EOpReportIntersection: return RValueOperator::ReportIntersection;
        case EOpIgnoreIntersectionNV: return RValueOperator::IgnoreIntersectionNV;
        case EOpTerminateRayNV: return RValueOperator::TerminateRayNV;
        case EOpExecuteCallableNV: return RValueOperator::ExecuteCallableNV;
        case EOpExecuteCallableKHR: return RValueOperator::ExecuteCallableKHR;
        case EOpWritePackedPrimitiveIndices4x8NV: return RValueOperator::WritePackedPrimitiveIndices4x8NV;
        case EOpEmitMeshTasksEXT: return RValueOperator::EmitMeshTasksEXT;
        case EOpSetMeshOutputsEXT: return RValueOperator::SetMeshOutputsEXT;
        case EOpRayQueryInitialize: return RValueOperator::RayQueryInitialize;
        case EOpRayQueryTerminate: return RValueOperator::RayQueryTerminate;
        case EOpRayQueryGenerateIntersection: return RValueOperator::RayQueryGenerateIntersection;
        case EOpRayQueryConfirmIntersection: return RValueOperator::RayQueryConfirmIntersection;
        case EOpRayQueryProceed: return RValueOperator::RayQueryProceed;
        case EOpRayQueryGetIntersectionType: return RValueOperator::RayQueryGetIntersectionType;
        case EOpRayQueryGetRayTMin: return RValueOperator::RayQueryGetRayTMin;
        case EOpRayQueryGetRayFlags: return RValueOperator::RayQueryGetRayFlags;
        case EOpRayQueryGetIntersectionT: return RValueOperator::RayQueryGetIntersectionT;
        case EOpRayQueryGetIntersectionInstanceCustomIndex: return RValueOperator::RayQueryGetIntersectionInstanceCustomIndex;
        case EOpRayQueryGetIntersectionInstanceId: return RValueOperator::RayQueryGetIntersectionInstanceId;
        case EOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffset: return RValueOperator::RayQueryGetIntersectionInstanceShaderBindingTableRecordOffset;
        case EOpRayQueryGetIntersectionGeometryIndex: return RValueOperator::RayQueryGetIntersectionGeometryIndex;
        case EOpRayQueryGetIntersectionPrimitiveIndex: return RValueOperator::RayQueryGetIntersectionPrimitiveIndex;
        case EOpRayQueryGetIntersectionBarycentrics: return RValueOperator::RayQueryGetIntersectionBarycentrics;
        case EOpRayQueryGetIntersectionFrontFace: return RValueOperator::RayQueryGetIntersectionFrontFace;
        case EOpRayQueryGetIntersectionCandidateAABBOpaque: return RValueOperator::RayQueryGetIntersectionCandidateAABBOpaque;
        case EOpRayQueryGetIntersectionObjectRayDirection: return RValueOperator::RayQueryGetIntersectionObjectRayDirection;
        case EOpRayQueryGetIntersectionObjectRayOrigin: return RValueOperator::RayQueryGetIntersectionObjectRayOrigin;
        case EOpRayQueryGetWorldRayDirection: return RValueOperator::RayQueryGetWorldRayDirection;
        case EOpRayQueryGetWorldRayOrigin: return RValueOperator::RayQueryGetWorldRayOrigin;
        case EOpRayQueryGetIntersectionObjectToWorld: return RValueOperator::RayQueryGetIntersectionObjectToWorld;
        case EOpRayQueryGetIntersectionWorldToObject: return RValueOperator::RayQueryGetIntersectionWorldToObject;
        case EOpHitObjectTraceRayNV: return RValueOperator::HitObjectTraceRayNV;
        case EOpHitObjectTraceRayMotionNV: return RValueOperator::HitObjectTraceRayMotionNV;
        case EOpHitObjectRecordHitNV: return RValueOperator::HitObjectRecordHitNV;
        case EOpHitObjectRecordHitMotionNV: return RValueOperator::HitObjectRecordHitMotionNV;
        case EOpHitObjectRecordHitWithIndexNV: return RValueOperator::HitObjectRecordHitWithIndexNV;
        case EOpHitObjectRecordHitWithIndexMotionNV: return RValueOperator::HitObjectRecordHitWithIndexMotionNV;
        case EOpHitObjectRecordMissNV: return RValueOperator::HitObjectRecordMissNV;
        case EOpHitObjectRecordMissMotionNV: return RValueOperator::HitObjectRecordMissMotionNV;
        case EOpHitObjectRecordEmptyNV: return RValueOperator::HitObjectRecordEmptyNV;
        case EOpHitObjectExecuteShaderNV: return RValueOperator::HitObjectExecuteShaderNV;
        case EOpHitObjectIsEmptyNV: return RValueOperator::HitObjectIsEmptyNV;
        case EOpHitObjectIsMissNV: return RValueOperator::HitObjectIsMissNV;
        case EOpHitObjectIsHitNV: return RValueOperator::HitObjectIsHitNV;
        case EOpHitObjectGetRayTMinNV: return RValueOperator::HitObjectGetRayTMinNV;
        case EOpHitObjectGetRayTMaxNV: return RValueOperator::HitObjectGetRayTMaxNV;
        case EOpHitObjectGetObjectRayOriginNV: return RValueOperator::HitObjectGetObjectRayOriginNV;
        case EOpHitObjectGetObjectRayDirectionNV: return RValueOperator::HitObjectGetObjectRayDirectionNV;
        case EOpHitObjectGetWorldRayOriginNV: return RValueOperator::HitObjectGetWorldRayOriginNV;
        case EOpHitObjectGetWorldRayDirectionNV: return RValueOperator::HitObjectGetWorldRayDirectionNV;
        case EOpHitObjectGetWorldToObjectNV: return RValueOperator::HitObjectGetWorldToObjectNV;
        case EOpHitObjectGetObjectToWorldNV: return RValueOperator::HitObjectGetObjectToWorldNV;
        case EOpHitObjectGetInstanceCustomIndexNV: return RValueOperator::HitObjectGetInstanceCustomIndexNV;
        case EOpHitObjectGetInstanceIdNV: return RValueOperator::HitObjectGetInstanceIdNV;
        case EOpHitObjectGetGeometryIndexNV: return RValueOperator::HitObjectGetGeometryIndexNV;
        case EOpHitObjectGetPrimitiveIndexNV: return RValueOperator::HitObjectGetPrimitiveIndexNV;
        case EOpHitObjectGetHitKindNV: return RValueOperator::HitObjectGetHitKindNV;
        case EOpHitObjectGetShaderBindingTableRecordIndexNV: return RValueOperator::HitObjectGetShaderBindingTableRecordIndexNV;
        case EOpHitObjectGetShaderRecordBufferHandleNV: return RValueOperator::HitObjectGetShaderRecordBufferHandleNV;
        case EOpHitObjectGetAttributesNV: return RValueOperator::HitObjectGetAttributesNV;
        case EOpHitObjectGetCurrentTimeNV: return RValueOperator::HitObjectGetCurrentTimeNV;
        case EOpReorderThreadNV: return RValueOperator::ReorderThreadNV;
        case EOpFetchMicroTriangleVertexPositionNV: return RValueOperator::FetchMicroTriangleVertexPositionNV;
        case EOpFetchMicroTriangleVertexBarycentricNV: return RValueOperator::FetchMicroTriangleVertexBarycentricNV;
        default:
            utils::slog.e << "Cannot convert operator " << operatorToString(op) << " to RValue operator.";
            // TODO: abort here
            return RValueOperator::Ternary;
    }
}

const char* rValueOperatorToString(RValueOperator op) {
    switch(op) {
        case RValueOperator::Ternary: return "Ternary";
        case RValueOperator::Negative: return "Negative";
        case RValueOperator::LogicalNot: return "LogicalNot";
        case RValueOperator::VectorLogicalNot: return "VectorLogicalNot";
        case RValueOperator::BitwiseNot: return "BitwiseNot";
        case RValueOperator::PostIncrement: return "PostIncrement";
        case RValueOperator::PostDecrement: return "PostDecrement";
        case RValueOperator::PreIncrement: return "PreIncrement";
        case RValueOperator::PreDecrement: return "PreDecrement";
        case RValueOperator::CopyObject: return "CopyObject";
        case RValueOperator::Declare: return "Declare";
        case RValueOperator::ConvInt8ToBool: return "ConvInt8ToBool";
        case RValueOperator::ConvUint8ToBool: return "ConvUint8ToBool";
        case RValueOperator::ConvInt16ToBool: return "ConvInt16ToBool";
        case RValueOperator::ConvUint16ToBool: return "ConvUint16ToBool";
        case RValueOperator::ConvIntToBool: return "ConvIntToBool";
        case RValueOperator::ConvUintToBool: return "ConvUintToBool";
        case RValueOperator::ConvInt64ToBool: return "ConvInt64ToBool";
        case RValueOperator::ConvUint64ToBool: return "ConvUint64ToBool";
        case RValueOperator::ConvFloat16ToBool: return "ConvFloat16ToBool";
        case RValueOperator::ConvFloatToBool: return "ConvFloatToBool";
        case RValueOperator::ConvDoubleToBool: return "ConvDoubleToBool";
        case RValueOperator::ConvBoolToInt8: return "ConvBoolToInt8";
        case RValueOperator::ConvBoolToUint8: return "ConvBoolToUint8";
        case RValueOperator::ConvBoolToInt16: return "ConvBoolToInt16";
        case RValueOperator::ConvBoolToUint16: return "ConvBoolToUint16";
        case RValueOperator::ConvBoolToInt: return "ConvBoolToInt";
        case RValueOperator::ConvBoolToUint: return "ConvBoolToUint";
        case RValueOperator::ConvBoolToInt64: return "ConvBoolToInt64";
        case RValueOperator::ConvBoolToUint64: return "ConvBoolToUint64";
        case RValueOperator::ConvBoolToFloat16: return "ConvBoolToFloat16";
        case RValueOperator::ConvBoolToFloat: return "ConvBoolToFloat";
        case RValueOperator::ConvBoolToDouble: return "ConvBoolToDouble";
        case RValueOperator::ConvInt8ToInt16: return "ConvInt8ToInt16";
        case RValueOperator::ConvInt8ToInt: return "ConvInt8ToInt";
        case RValueOperator::ConvInt8ToInt64: return "ConvInt8ToInt64";
        case RValueOperator::ConvInt8ToUint8: return "ConvInt8ToUint8";
        case RValueOperator::ConvInt8ToUint16: return "ConvInt8ToUint16";
        case RValueOperator::ConvInt8ToUint: return "ConvInt8ToUint";
        case RValueOperator::ConvInt8ToUint64: return "ConvInt8ToUint64";
        case RValueOperator::ConvUint8ToInt8: return "ConvUint8ToInt8";
        case RValueOperator::ConvUint8ToInt16: return "ConvUint8ToInt16";
        case RValueOperator::ConvUint8ToInt: return "ConvUint8ToInt";
        case RValueOperator::ConvUint8ToInt64: return "ConvUint8ToInt64";
        case RValueOperator::ConvUint8ToUint16: return "ConvUint8ToUint16";
        case RValueOperator::ConvUint8ToUint: return "ConvUint8ToUint";
        case RValueOperator::ConvUint8ToUint64: return "ConvUint8ToUint64";
        case RValueOperator::ConvInt8ToFloat16: return "ConvInt8ToFloat16";
        case RValueOperator::ConvInt8ToFloat: return "ConvInt8ToFloat";
        case RValueOperator::ConvInt8ToDouble: return "ConvInt8ToDouble";
        case RValueOperator::ConvUint8ToFloat16: return "ConvUint8ToFloat16";
        case RValueOperator::ConvUint8ToFloat: return "ConvUint8ToFloat";
        case RValueOperator::ConvUint8ToDouble: return "ConvUint8ToDouble";
        case RValueOperator::ConvInt16ToInt8: return "ConvInt16ToInt8";
        case RValueOperator::ConvInt16ToInt: return "ConvInt16ToInt";
        case RValueOperator::ConvInt16ToInt64: return "ConvInt16ToInt64";
        case RValueOperator::ConvInt16ToUint8: return "ConvInt16ToUint8";
        case RValueOperator::ConvInt16ToUint16: return "ConvInt16ToUint16";
        case RValueOperator::ConvInt16ToUint: return "ConvInt16ToUint";
        case RValueOperator::ConvInt16ToUint64: return "ConvInt16ToUint64";
        case RValueOperator::ConvUint16ToInt8: return "ConvUint16ToInt8";
        case RValueOperator::ConvUint16ToInt16: return "ConvUint16ToInt16";
        case RValueOperator::ConvUint16ToInt: return "ConvUint16ToInt";
        case RValueOperator::ConvUint16ToInt64: return "ConvUint16ToInt64";
        case RValueOperator::ConvUint16ToUint8: return "ConvUint16ToUint8";
        case RValueOperator::ConvUint16ToUint: return "ConvUint16ToUint";
        case RValueOperator::ConvUint16ToUint64: return "ConvUint16ToUint64";
        case RValueOperator::ConvInt16ToFloat16: return "ConvInt16ToFloat16";
        case RValueOperator::ConvInt16ToFloat: return "ConvInt16ToFloat";
        case RValueOperator::ConvInt16ToDouble: return "ConvInt16ToDouble";
        case RValueOperator::ConvUint16ToFloat16: return "ConvUint16ToFloat16";
        case RValueOperator::ConvUint16ToFloat: return "ConvUint16ToFloat";
        case RValueOperator::ConvUint16ToDouble: return "ConvUint16ToDouble";
        case RValueOperator::ConvIntToInt8: return "ConvIntToInt8";
        case RValueOperator::ConvIntToInt16: return "ConvIntToInt16";
        case RValueOperator::ConvIntToInt64: return "ConvIntToInt64";
        case RValueOperator::ConvIntToUint8: return "ConvIntToUint8";
        case RValueOperator::ConvIntToUint16: return "ConvIntToUint16";
        case RValueOperator::ConvIntToUint: return "ConvIntToUint";
        case RValueOperator::ConvIntToUint64: return "ConvIntToUint64";
        case RValueOperator::ConvUintToInt8: return "ConvUintToInt8";
        case RValueOperator::ConvUintToInt16: return "ConvUintToInt16";
        case RValueOperator::ConvUintToInt: return "ConvUintToInt";
        case RValueOperator::ConvUintToInt64: return "ConvUintToInt64";
        case RValueOperator::ConvUintToUint8: return "ConvUintToUint8";
        case RValueOperator::ConvUintToUint16: return "ConvUintToUint16";
        case RValueOperator::ConvUintToUint64: return "ConvUintToUint64";
        case RValueOperator::ConvIntToFloat16: return "ConvIntToFloat16";
        case RValueOperator::ConvIntToFloat: return "ConvIntToFloat";
        case RValueOperator::ConvIntToDouble: return "ConvIntToDouble";
        case RValueOperator::ConvUintToFloat16: return "ConvUintToFloat16";
        case RValueOperator::ConvUintToFloat: return "ConvUintToFloat";
        case RValueOperator::ConvUintToDouble: return "ConvUintToDouble";
        case RValueOperator::ConvInt64ToInt8: return "ConvInt64ToInt8";
        case RValueOperator::ConvInt64ToInt16: return "ConvInt64ToInt16";
        case RValueOperator::ConvInt64ToInt: return "ConvInt64ToInt";
        case RValueOperator::ConvInt64ToUint8: return "ConvInt64ToUint8";
        case RValueOperator::ConvInt64ToUint16: return "ConvInt64ToUint16";
        case RValueOperator::ConvInt64ToUint: return "ConvInt64ToUint";
        case RValueOperator::ConvInt64ToUint64: return "ConvInt64ToUint64";
        case RValueOperator::ConvUint64ToInt8: return "ConvUint64ToInt8";
        case RValueOperator::ConvUint64ToInt16: return "ConvUint64ToInt16";
        case RValueOperator::ConvUint64ToInt: return "ConvUint64ToInt";
        case RValueOperator::ConvUint64ToInt64: return "ConvUint64ToInt64";
        case RValueOperator::ConvUint64ToUint8: return "ConvUint64ToUint8";
        case RValueOperator::ConvUint64ToUint16: return "ConvUint64ToUint16";
        case RValueOperator::ConvUint64ToUint: return "ConvUint64ToUint";
        case RValueOperator::ConvInt64ToFloat16: return "ConvInt64ToFloat16";
        case RValueOperator::ConvInt64ToFloat: return "ConvInt64ToFloat";
        case RValueOperator::ConvInt64ToDouble: return "ConvInt64ToDouble";
        case RValueOperator::ConvUint64ToFloat16: return "ConvUint64ToFloat16";
        case RValueOperator::ConvUint64ToFloat: return "ConvUint64ToFloat";
        case RValueOperator::ConvUint64ToDouble: return "ConvUint64ToDouble";
        case RValueOperator::ConvFloat16ToInt8: return "ConvFloat16ToInt8";
        case RValueOperator::ConvFloat16ToInt16: return "ConvFloat16ToInt16";
        case RValueOperator::ConvFloat16ToInt: return "ConvFloat16ToInt";
        case RValueOperator::ConvFloat16ToInt64: return "ConvFloat16ToInt64";
        case RValueOperator::ConvFloat16ToUint8: return "ConvFloat16ToUint8";
        case RValueOperator::ConvFloat16ToUint16: return "ConvFloat16ToUint16";
        case RValueOperator::ConvFloat16ToUint: return "ConvFloat16ToUint";
        case RValueOperator::ConvFloat16ToUint64: return "ConvFloat16ToUint64";
        case RValueOperator::ConvFloat16ToFloat: return "ConvFloat16ToFloat";
        case RValueOperator::ConvFloat16ToDouble: return "ConvFloat16ToDouble";
        case RValueOperator::ConvFloatToInt8: return "ConvFloatToInt8";
        case RValueOperator::ConvFloatToInt16: return "ConvFloatToInt16";
        case RValueOperator::ConvFloatToInt: return "ConvFloatToInt";
        case RValueOperator::ConvFloatToInt64: return "ConvFloatToInt64";
        case RValueOperator::ConvFloatToUint8: return "ConvFloatToUint8";
        case RValueOperator::ConvFloatToUint16: return "ConvFloatToUint16";
        case RValueOperator::ConvFloatToUint: return "ConvFloatToUint";
        case RValueOperator::ConvFloatToUint64: return "ConvFloatToUint64";
        case RValueOperator::ConvFloatToFloat16: return "ConvFloatToFloat16";
        case RValueOperator::ConvFloatToDouble: return "ConvFloatToDouble";
        case RValueOperator::ConvDoubleToInt8: return "ConvDoubleToInt8";
        case RValueOperator::ConvDoubleToInt16: return "ConvDoubleToInt16";
        case RValueOperator::ConvDoubleToInt: return "ConvDoubleToInt";
        case RValueOperator::ConvDoubleToInt64: return "ConvDoubleToInt64";
        case RValueOperator::ConvDoubleToUint8: return "ConvDoubleToUint8";
        case RValueOperator::ConvDoubleToUint16: return "ConvDoubleToUint16";
        case RValueOperator::ConvDoubleToUint: return "ConvDoubleToUint";
        case RValueOperator::ConvDoubleToUint64: return "ConvDoubleToUint64";
        case RValueOperator::ConvDoubleToFloat16: return "ConvDoubleToFloat16";
        case RValueOperator::ConvDoubleToFloat: return "ConvDoubleToFloat";
        case RValueOperator::ConvUint64ToPtr: return "ConvUint64ToPtr";
        case RValueOperator::ConvPtrToUint64: return "ConvPtrToUint64";
        case RValueOperator::ConvUvec2ToPtr: return "ConvUvec2ToPtr";
        case RValueOperator::ConvPtrToUvec2: return "ConvPtrToUvec2";
        case RValueOperator::ConvUint64ToAccStruct: return "ConvUint64ToAccStruct";
        case RValueOperator::ConvUvec2ToAccStruct: return "ConvUvec2ToAccStruct";
        case RValueOperator::Add: return "Add";
        case RValueOperator::Sub: return "Sub";
        case RValueOperator::Mul: return "Mul";
        case RValueOperator::Div: return "Div";
        case RValueOperator::Mod: return "Mod";
        case RValueOperator::RightShift: return "RightShift";
        case RValueOperator::LeftShift: return "LeftShift";
        case RValueOperator::And: return "And";
        case RValueOperator::InclusiveOr: return "InclusiveOr";
        case RValueOperator::ExclusiveOr: return "ExclusiveOr";
        case RValueOperator::Equal: return "Equal";
        case RValueOperator::NotEqual: return "NotEqual";
        case RValueOperator::VectorEqual: return "VectorEqual";
        case RValueOperator::VectorNotEqual: return "VectorNotEqual";
        case RValueOperator::LessThan: return "LessThan";
        case RValueOperator::GreaterThan: return "GreaterThan";
        case RValueOperator::LessThanEqual: return "LessThanEqual";
        case RValueOperator::GreaterThanEqual: return "GreaterThanEqual";
        case RValueOperator::Comma: return "Comma";
        case RValueOperator::VectorTimesScalar: return "VectorTimesScalar";
        case RValueOperator::VectorTimesMatrix: return "VectorTimesMatrix";
        case RValueOperator::MatrixTimesVector: return "MatrixTimesVector";
        case RValueOperator::MatrixTimesScalar: return "MatrixTimesScalar";
        case RValueOperator::LogicalOr: return "LogicalOr";
        case RValueOperator::LogicalXor: return "LogicalXor";
        case RValueOperator::LogicalAnd: return "LogicalAnd";
        case RValueOperator::IndexDirect: return "IndexDirect";
        case RValueOperator::IndexIndirect: return "IndexIndirect";
        case RValueOperator::IndexDirectStruct: return "IndexDirectStruct";
        case RValueOperator::VectorSwizzle: return "VectorSwizzle";
        case RValueOperator::Method: return "Method";
        case RValueOperator::Scoping: return "Scoping";
        case RValueOperator::Radians: return "Radians";
        case RValueOperator::Degrees: return "Degrees";
        case RValueOperator::Sin: return "Sin";
        case RValueOperator::Cos: return "Cos";
        case RValueOperator::Tan: return "Tan";
        case RValueOperator::Asin: return "Asin";
        case RValueOperator::Acos: return "Acos";
        case RValueOperator::Atan: return "Atan";
        case RValueOperator::Sinh: return "Sinh";
        case RValueOperator::Cosh: return "Cosh";
        case RValueOperator::Tanh: return "Tanh";
        case RValueOperator::Asinh: return "Asinh";
        case RValueOperator::Acosh: return "Acosh";
        case RValueOperator::Atanh: return "Atanh";
        case RValueOperator::Pow: return "Pow";
        case RValueOperator::Exp: return "Exp";
        case RValueOperator::Log: return "Log";
        case RValueOperator::Exp2: return "Exp2";
        case RValueOperator::Log2: return "Log2";
        case RValueOperator::Sqrt: return "Sqrt";
        case RValueOperator::InverseSqrt: return "InverseSqrt";
        case RValueOperator::Abs: return "Abs";
        case RValueOperator::Sign: return "Sign";
        case RValueOperator::Floor: return "Floor";
        case RValueOperator::Trunc: return "Trunc";
        case RValueOperator::Round: return "Round";
        case RValueOperator::RoundEven: return "RoundEven";
        case RValueOperator::Ceil: return "Ceil";
        case RValueOperator::Fract: return "Fract";
        case RValueOperator::Modf: return "Modf";
        case RValueOperator::Min: return "Min";
        case RValueOperator::Max: return "Max";
        case RValueOperator::Clamp: return "Clamp";
        case RValueOperator::Mix: return "Mix";
        case RValueOperator::Step: return "Step";
        case RValueOperator::SmoothStep: return "SmoothStep";
        case RValueOperator::IsNan: return "IsNan";
        case RValueOperator::IsInf: return "IsInf";
        case RValueOperator::Fma: return "Fma";
        case RValueOperator::Frexp: return "Frexp";
        case RValueOperator::Ldexp: return "Ldexp";
        case RValueOperator::FloatBitsToInt: return "FloatBitsToInt";
        case RValueOperator::FloatBitsToUint: return "FloatBitsToUint";
        case RValueOperator::IntBitsToFloat: return "IntBitsToFloat";
        case RValueOperator::UintBitsToFloat: return "UintBitsToFloat";
        case RValueOperator::DoubleBitsToInt64: return "DoubleBitsToInt64";
        case RValueOperator::DoubleBitsToUint64: return "DoubleBitsToUint64";
        case RValueOperator::Int64BitsToDouble: return "Int64BitsToDouble";
        case RValueOperator::Uint64BitsToDouble: return "Uint64BitsToDouble";
        case RValueOperator::Float16BitsToInt16: return "Float16BitsToInt16";
        case RValueOperator::Float16BitsToUint16: return "Float16BitsToUint16";
        case RValueOperator::Int16BitsToFloat16: return "Int16BitsToFloat16";
        case RValueOperator::Uint16BitsToFloat16: return "Uint16BitsToFloat16";
        case RValueOperator::PackSnorm2x16: return "PackSnorm2x16";
        case RValueOperator::UnpackSnorm2x16: return "UnpackSnorm2x16";
        case RValueOperator::PackUnorm2x16: return "PackUnorm2x16";
        case RValueOperator::UnpackUnorm2x16: return "UnpackUnorm2x16";
        case RValueOperator::PackSnorm4x8: return "PackSnorm4x8";
        case RValueOperator::UnpackSnorm4x8: return "UnpackSnorm4x8";
        case RValueOperator::PackUnorm4x8: return "PackUnorm4x8";
        case RValueOperator::UnpackUnorm4x8: return "UnpackUnorm4x8";
        case RValueOperator::PackHalf2x16: return "PackHalf2x16";
        case RValueOperator::UnpackHalf2x16: return "UnpackHalf2x16";
        case RValueOperator::PackDouble2x32: return "PackDouble2x32";
        case RValueOperator::UnpackDouble2x32: return "UnpackDouble2x32";
        case RValueOperator::PackInt2x32: return "PackInt2x32";
        case RValueOperator::UnpackInt2x32: return "UnpackInt2x32";
        case RValueOperator::PackUint2x32: return "PackUint2x32";
        case RValueOperator::UnpackUint2x32: return "UnpackUint2x32";
        case RValueOperator::PackFloat2x16: return "PackFloat2x16";
        case RValueOperator::UnpackFloat2x16: return "UnpackFloat2x16";
        case RValueOperator::PackInt2x16: return "PackInt2x16";
        case RValueOperator::UnpackInt2x16: return "UnpackInt2x16";
        case RValueOperator::PackUint2x16: return "PackUint2x16";
        case RValueOperator::UnpackUint2x16: return "UnpackUint2x16";
        case RValueOperator::PackInt4x16: return "PackInt4x16";
        case RValueOperator::UnpackInt4x16: return "UnpackInt4x16";
        case RValueOperator::PackUint4x16: return "PackUint4x16";
        case RValueOperator::UnpackUint4x16: return "UnpackUint4x16";
        case RValueOperator::Pack16: return "Pack16";
        case RValueOperator::Pack32: return "Pack32";
        case RValueOperator::Pack64: return "Pack64";
        case RValueOperator::Unpack32: return "Unpack32";
        case RValueOperator::Unpack16: return "Unpack16";
        case RValueOperator::Unpack8: return "Unpack8";
        case RValueOperator::Length: return "Length";
        case RValueOperator::Distance: return "Distance";
        case RValueOperator::Dot: return "Dot";
        case RValueOperator::Cross: return "Cross";
        case RValueOperator::Normalize: return "Normalize";
        case RValueOperator::FaceForward: return "FaceForward";
        case RValueOperator::Reflect: return "Reflect";
        case RValueOperator::Refract: return "Refract";
        case RValueOperator::Min3: return "Min3";
        case RValueOperator::Max3: return "Max3";
        case RValueOperator::Mid3: return "Mid3";
        case RValueOperator::DPdx: return "DPdx";
        case RValueOperator::DPdy: return "DPdy";
        case RValueOperator::Fwidth: return "Fwidth";
        case RValueOperator::DPdxFine: return "DPdxFine";
        case RValueOperator::DPdyFine: return "DPdyFine";
        case RValueOperator::FwidthFine: return "FwidthFine";
        case RValueOperator::DPdxCoarse: return "DPdxCoarse";
        case RValueOperator::DPdyCoarse: return "DPdyCoarse";
        case RValueOperator::FwidthCoarse: return "FwidthCoarse";
        case RValueOperator::InterpolateAtCentroid: return "InterpolateAtCentroid";
        case RValueOperator::InterpolateAtSample: return "InterpolateAtSample";
        case RValueOperator::InterpolateAtOffset: return "InterpolateAtOffset";
        case RValueOperator::InterpolateAtVertex: return "InterpolateAtVertex";
        case RValueOperator::MatrixTimesMatrix: return "MatrixTimesMatrix";
        case RValueOperator::OuterProduct: return "OuterProduct";
        case RValueOperator::Determinant: return "Determinant";
        case RValueOperator::MatrixInverse: return "MatrixInverse";
        case RValueOperator::Transpose: return "Transpose";
        case RValueOperator::Ftransform: return "Ftransform";
        case RValueOperator::Noise: return "Noise";
        case RValueOperator::EmitVertex: return "EmitVertex";
        case RValueOperator::EndPrimitive: return "EndPrimitive";
        case RValueOperator::EmitStreamVertex: return "EmitStreamVertex";
        case RValueOperator::EndStreamPrimitive: return "EndStreamPrimitive";
        case RValueOperator::Barrier: return "Barrier";
        case RValueOperator::MemoryBarrier: return "MemoryBarrier";
        case RValueOperator::MemoryBarrierAtomicCounter: return "MemoryBarrierAtomicCounter";
        case RValueOperator::MemoryBarrierBuffer: return "MemoryBarrierBuffer";
        case RValueOperator::MemoryBarrierImage: return "MemoryBarrierImage";
        case RValueOperator::MemoryBarrierShared: return "MemoryBarrierShared";
        case RValueOperator::GroupMemoryBarrier: return "GroupMemoryBarrier";
        case RValueOperator::Ballot: return "Ballot";
        case RValueOperator::ReadInvocation: return "ReadInvocation";
        case RValueOperator::ReadFirstInvocation: return "ReadFirstInvocation";
        case RValueOperator::AnyInvocation: return "AnyInvocation";
        case RValueOperator::AllInvocations: return "AllInvocations";
        case RValueOperator::AllInvocationsEqual: return "AllInvocationsEqual";
        case RValueOperator::SubgroupGuardStart: return "SubgroupGuardStart";
        case RValueOperator::SubgroupBarrier: return "SubgroupBarrier";
        case RValueOperator::SubgroupMemoryBarrier: return "SubgroupMemoryBarrier";
        case RValueOperator::SubgroupMemoryBarrierBuffer: return "SubgroupMemoryBarrierBuffer";
        case RValueOperator::SubgroupMemoryBarrierImage: return "SubgroupMemoryBarrierImage";
        case RValueOperator::SubgroupMemoryBarrierShared: return "SubgroupMemoryBarrierShared";
        case RValueOperator::SubgroupElect: return "SubgroupElect";
        case RValueOperator::SubgroupAll: return "SubgroupAll";
        case RValueOperator::SubgroupAny: return "SubgroupAny";
        case RValueOperator::SubgroupAllEqual: return "SubgroupAllEqual";
        case RValueOperator::SubgroupBroadcast: return "SubgroupBroadcast";
        case RValueOperator::SubgroupBroadcastFirst: return "SubgroupBroadcastFirst";
        case RValueOperator::SubgroupBallot: return "SubgroupBallot";
        case RValueOperator::SubgroupInverseBallot: return "SubgroupInverseBallot";
        case RValueOperator::SubgroupBallotBitExtract: return "SubgroupBallotBitExtract";
        case RValueOperator::SubgroupBallotBitCount: return "SubgroupBallotBitCount";
        case RValueOperator::SubgroupBallotInclusiveBitCount: return "SubgroupBallotInclusiveBitCount";
        case RValueOperator::SubgroupBallotExclusiveBitCount: return "SubgroupBallotExclusiveBitCount";
        case RValueOperator::SubgroupBallotFindLSB: return "SubgroupBallotFindLSB";
        case RValueOperator::SubgroupBallotFindMSB: return "SubgroupBallotFindMSB";
        case RValueOperator::SubgroupShuffle: return "SubgroupShuffle";
        case RValueOperator::SubgroupShuffleXor: return "SubgroupShuffleXor";
        case RValueOperator::SubgroupShuffleUp: return "SubgroupShuffleUp";
        case RValueOperator::SubgroupShuffleDown: return "SubgroupShuffleDown";
        case RValueOperator::SubgroupAdd: return "SubgroupAdd";
        case RValueOperator::SubgroupMul: return "SubgroupMul";
        case RValueOperator::SubgroupMin: return "SubgroupMin";
        case RValueOperator::SubgroupMax: return "SubgroupMax";
        case RValueOperator::SubgroupAnd: return "SubgroupAnd";
        case RValueOperator::SubgroupOr: return "SubgroupOr";
        case RValueOperator::SubgroupXor: return "SubgroupXor";
        case RValueOperator::SubgroupInclusiveAdd: return "SubgroupInclusiveAdd";
        case RValueOperator::SubgroupInclusiveMul: return "SubgroupInclusiveMul";
        case RValueOperator::SubgroupInclusiveMin: return "SubgroupInclusiveMin";
        case RValueOperator::SubgroupInclusiveMax: return "SubgroupInclusiveMax";
        case RValueOperator::SubgroupInclusiveAnd: return "SubgroupInclusiveAnd";
        case RValueOperator::SubgroupInclusiveOr: return "SubgroupInclusiveOr";
        case RValueOperator::SubgroupInclusiveXor: return "SubgroupInclusiveXor";
        case RValueOperator::SubgroupExclusiveAdd: return "SubgroupExclusiveAdd";
        case RValueOperator::SubgroupExclusiveMul: return "SubgroupExclusiveMul";
        case RValueOperator::SubgroupExclusiveMin: return "SubgroupExclusiveMin";
        case RValueOperator::SubgroupExclusiveMax: return "SubgroupExclusiveMax";
        case RValueOperator::SubgroupExclusiveAnd: return "SubgroupExclusiveAnd";
        case RValueOperator::SubgroupExclusiveOr: return "SubgroupExclusiveOr";
        case RValueOperator::SubgroupExclusiveXor: return "SubgroupExclusiveXor";
        case RValueOperator::SubgroupClusteredAdd: return "SubgroupClusteredAdd";
        case RValueOperator::SubgroupClusteredMul: return "SubgroupClusteredMul";
        case RValueOperator::SubgroupClusteredMin: return "SubgroupClusteredMin";
        case RValueOperator::SubgroupClusteredMax: return "SubgroupClusteredMax";
        case RValueOperator::SubgroupClusteredAnd: return "SubgroupClusteredAnd";
        case RValueOperator::SubgroupClusteredOr: return "SubgroupClusteredOr";
        case RValueOperator::SubgroupClusteredXor: return "SubgroupClusteredXor";
        case RValueOperator::SubgroupQuadBroadcast: return "SubgroupQuadBroadcast";
        case RValueOperator::SubgroupQuadSwapHorizontal: return "SubgroupQuadSwapHorizontal";
        case RValueOperator::SubgroupQuadSwapVertical: return "SubgroupQuadSwapVertical";
        case RValueOperator::SubgroupQuadSwapDiagonal: return "SubgroupQuadSwapDiagonal";
        case RValueOperator::SubgroupPartition: return "SubgroupPartition";
        case RValueOperator::SubgroupPartitionedAdd: return "SubgroupPartitionedAdd";
        case RValueOperator::SubgroupPartitionedMul: return "SubgroupPartitionedMul";
        case RValueOperator::SubgroupPartitionedMin: return "SubgroupPartitionedMin";
        case RValueOperator::SubgroupPartitionedMax: return "SubgroupPartitionedMax";
        case RValueOperator::SubgroupPartitionedAnd: return "SubgroupPartitionedAnd";
        case RValueOperator::SubgroupPartitionedOr: return "SubgroupPartitionedOr";
        case RValueOperator::SubgroupPartitionedXor: return "SubgroupPartitionedXor";
        case RValueOperator::SubgroupPartitionedInclusiveAdd: return "SubgroupPartitionedInclusiveAdd";
        case RValueOperator::SubgroupPartitionedInclusiveMul: return "SubgroupPartitionedInclusiveMul";
        case RValueOperator::SubgroupPartitionedInclusiveMin: return "SubgroupPartitionedInclusiveMin";
        case RValueOperator::SubgroupPartitionedInclusiveMax: return "SubgroupPartitionedInclusiveMax";
        case RValueOperator::SubgroupPartitionedInclusiveAnd: return "SubgroupPartitionedInclusiveAnd";
        case RValueOperator::SubgroupPartitionedInclusiveOr: return "SubgroupPartitionedInclusiveOr";
        case RValueOperator::SubgroupPartitionedInclusiveXor: return "SubgroupPartitionedInclusiveXor";
        case RValueOperator::SubgroupPartitionedExclusiveAdd: return "SubgroupPartitionedExclusiveAdd";
        case RValueOperator::SubgroupPartitionedExclusiveMul: return "SubgroupPartitionedExclusiveMul";
        case RValueOperator::SubgroupPartitionedExclusiveMin: return "SubgroupPartitionedExclusiveMin";
        case RValueOperator::SubgroupPartitionedExclusiveMax: return "SubgroupPartitionedExclusiveMax";
        case RValueOperator::SubgroupPartitionedExclusiveAnd: return "SubgroupPartitionedExclusiveAnd";
        case RValueOperator::SubgroupPartitionedExclusiveOr: return "SubgroupPartitionedExclusiveOr";
        case RValueOperator::SubgroupPartitionedExclusiveXor: return "SubgroupPartitionedExclusiveXor";
        case RValueOperator::SubgroupGuardStop: return "SubgroupGuardStop";
        case RValueOperator::MinInvocations: return "MinInvocations";
        case RValueOperator::MaxInvocations: return "MaxInvocations";
        case RValueOperator::AddInvocations: return "AddInvocations";
        case RValueOperator::MinInvocationsNonUniform: return "MinInvocationsNonUniform";
        case RValueOperator::MaxInvocationsNonUniform: return "MaxInvocationsNonUniform";
        case RValueOperator::AddInvocationsNonUniform: return "AddInvocationsNonUniform";
        case RValueOperator::MinInvocationsInclusiveScan: return "MinInvocationsInclusiveScan";
        case RValueOperator::MaxInvocationsInclusiveScan: return "MaxInvocationsInclusiveScan";
        case RValueOperator::AddInvocationsInclusiveScan: return "AddInvocationsInclusiveScan";
        case RValueOperator::MinInvocationsInclusiveScanNonUniform: return "MinInvocationsInclusiveScanNonUniform";
        case RValueOperator::MaxInvocationsInclusiveScanNonUniform: return "MaxInvocationsInclusiveScanNonUniform";
        case RValueOperator::AddInvocationsInclusiveScanNonUniform: return "AddInvocationsInclusiveScanNonUniform";
        case RValueOperator::MinInvocationsExclusiveScan: return "MinInvocationsExclusiveScan";
        case RValueOperator::MaxInvocationsExclusiveScan: return "MaxInvocationsExclusiveScan";
        case RValueOperator::AddInvocationsExclusiveScan: return "AddInvocationsExclusiveScan";
        case RValueOperator::MinInvocationsExclusiveScanNonUniform: return "MinInvocationsExclusiveScanNonUniform";
        case RValueOperator::MaxInvocationsExclusiveScanNonUniform: return "MaxInvocationsExclusiveScanNonUniform";
        case RValueOperator::AddInvocationsExclusiveScanNonUniform: return "AddInvocationsExclusiveScanNonUniform";
        case RValueOperator::SwizzleInvocations: return "SwizzleInvocations";
        case RValueOperator::SwizzleInvocationsMasked: return "SwizzleInvocationsMasked";
        case RValueOperator::WriteInvocation: return "WriteInvocation";
        case RValueOperator::Mbcnt: return "Mbcnt";
        case RValueOperator::CubeFaceIndex: return "CubeFaceIndex";
        case RValueOperator::CubeFaceCoord: return "CubeFaceCoord";
        case RValueOperator::Time: return "Time";
        case RValueOperator::AtomicAdd: return "AtomicAdd";
        case RValueOperator::AtomicSubtract: return "AtomicSubtract";
        case RValueOperator::AtomicMin: return "AtomicMin";
        case RValueOperator::AtomicMax: return "AtomicMax";
        case RValueOperator::AtomicAnd: return "AtomicAnd";
        case RValueOperator::AtomicOr: return "AtomicOr";
        case RValueOperator::AtomicXor: return "AtomicXor";
        case RValueOperator::AtomicExchange: return "AtomicExchange";
        case RValueOperator::AtomicCompSwap: return "AtomicCompSwap";
        case RValueOperator::AtomicLoad: return "AtomicLoad";
        case RValueOperator::AtomicStore: return "AtomicStore";
        case RValueOperator::AtomicCounterIncrement: return "AtomicCounterIncrement";
        case RValueOperator::AtomicCounterDecrement: return "AtomicCounterDecrement";
        case RValueOperator::AtomicCounter: return "AtomicCounter";
        case RValueOperator::AtomicCounterAdd: return "AtomicCounterAdd";
        case RValueOperator::AtomicCounterSubtract: return "AtomicCounterSubtract";
        case RValueOperator::AtomicCounterMin: return "AtomicCounterMin";
        case RValueOperator::AtomicCounterMax: return "AtomicCounterMax";
        case RValueOperator::AtomicCounterAnd: return "AtomicCounterAnd";
        case RValueOperator::AtomicCounterOr: return "AtomicCounterOr";
        case RValueOperator::AtomicCounterXor: return "AtomicCounterXor";
        case RValueOperator::AtomicCounterExchange: return "AtomicCounterExchange";
        case RValueOperator::AtomicCounterCompSwap: return "AtomicCounterCompSwap";
        case RValueOperator::Any: return "Any";
        case RValueOperator::All: return "All";
        case RValueOperator::CooperativeMatrixLoad: return "CooperativeMatrixLoad";
        case RValueOperator::CooperativeMatrixStore: return "CooperativeMatrixStore";
        case RValueOperator::CooperativeMatrixMulAdd: return "CooperativeMatrixMulAdd";
        case RValueOperator::CooperativeMatrixLoadNV: return "CooperativeMatrixLoadNV";
        case RValueOperator::CooperativeMatrixStoreNV: return "CooperativeMatrixStoreNV";
        case RValueOperator::CooperativeMatrixMulAddNV: return "CooperativeMatrixMulAddNV";
        case RValueOperator::BeginInvocationInterlock: return "BeginInvocationInterlock";
        case RValueOperator::EndInvocationInterlock: return "EndInvocationInterlock";
        case RValueOperator::IsHelperInvocation: return "IsHelperInvocation";
        case RValueOperator::DebugPrintf: return "DebugPrintf";
        case RValueOperator::ConstructGuardStart: return "ConstructGuardStart";
        case RValueOperator::ConstructInt: return "ConstructInt";
        case RValueOperator::ConstructUint: return "ConstructUint";
        case RValueOperator::ConstructInt8: return "ConstructInt8";
        case RValueOperator::ConstructUint8: return "ConstructUint8";
        case RValueOperator::ConstructInt16: return "ConstructInt16";
        case RValueOperator::ConstructUint16: return "ConstructUint16";
        case RValueOperator::ConstructInt64: return "ConstructInt64";
        case RValueOperator::ConstructUint64: return "ConstructUint64";
        case RValueOperator::ConstructBool: return "ConstructBool";
        case RValueOperator::ConstructFloat: return "ConstructFloat";
        case RValueOperator::ConstructDouble: return "ConstructDouble";
        case RValueOperator::ConstructVec2: return "ConstructVec2";
        case RValueOperator::ConstructVec3: return "ConstructVec3";
        case RValueOperator::ConstructVec4: return "ConstructVec4";
        case RValueOperator::ConstructMat2x2: return "ConstructMat2x2";
        case RValueOperator::ConstructMat2x3: return "ConstructMat2x3";
        case RValueOperator::ConstructMat2x4: return "ConstructMat2x4";
        case RValueOperator::ConstructMat3x2: return "ConstructMat3x2";
        case RValueOperator::ConstructMat3x3: return "ConstructMat3x3";
        case RValueOperator::ConstructMat3x4: return "ConstructMat3x4";
        case RValueOperator::ConstructMat4x2: return "ConstructMat4x2";
        case RValueOperator::ConstructMat4x3: return "ConstructMat4x3";
        case RValueOperator::ConstructMat4x4: return "ConstructMat4x4";
        case RValueOperator::ConstructDVec2: return "ConstructDVec2";
        case RValueOperator::ConstructDVec3: return "ConstructDVec3";
        case RValueOperator::ConstructDVec4: return "ConstructDVec4";
        case RValueOperator::ConstructBVec2: return "ConstructBVec2";
        case RValueOperator::ConstructBVec3: return "ConstructBVec3";
        case RValueOperator::ConstructBVec4: return "ConstructBVec4";
        case RValueOperator::ConstructI8Vec2: return "ConstructI8Vec2";
        case RValueOperator::ConstructI8Vec3: return "ConstructI8Vec3";
        case RValueOperator::ConstructI8Vec4: return "ConstructI8Vec4";
        case RValueOperator::ConstructU8Vec2: return "ConstructU8Vec2";
        case RValueOperator::ConstructU8Vec3: return "ConstructU8Vec3";
        case RValueOperator::ConstructU8Vec4: return "ConstructU8Vec4";
        case RValueOperator::ConstructI16Vec2: return "ConstructI16Vec2";
        case RValueOperator::ConstructI16Vec3: return "ConstructI16Vec3";
        case RValueOperator::ConstructI16Vec4: return "ConstructI16Vec4";
        case RValueOperator::ConstructU16Vec2: return "ConstructU16Vec2";
        case RValueOperator::ConstructU16Vec3: return "ConstructU16Vec3";
        case RValueOperator::ConstructU16Vec4: return "ConstructU16Vec4";
        case RValueOperator::ConstructIVec2: return "ConstructIVec2";
        case RValueOperator::ConstructIVec3: return "ConstructIVec3";
        case RValueOperator::ConstructIVec4: return "ConstructIVec4";
        case RValueOperator::ConstructUVec2: return "ConstructUVec2";
        case RValueOperator::ConstructUVec3: return "ConstructUVec3";
        case RValueOperator::ConstructUVec4: return "ConstructUVec4";
        case RValueOperator::ConstructI64Vec2: return "ConstructI64Vec2";
        case RValueOperator::ConstructI64Vec3: return "ConstructI64Vec3";
        case RValueOperator::ConstructI64Vec4: return "ConstructI64Vec4";
        case RValueOperator::ConstructU64Vec2: return "ConstructU64Vec2";
        case RValueOperator::ConstructU64Vec3: return "ConstructU64Vec3";
        case RValueOperator::ConstructU64Vec4: return "ConstructU64Vec4";
        case RValueOperator::ConstructDMat2x2: return "ConstructDMat2x2";
        case RValueOperator::ConstructDMat2x3: return "ConstructDMat2x3";
        case RValueOperator::ConstructDMat2x4: return "ConstructDMat2x4";
        case RValueOperator::ConstructDMat3x2: return "ConstructDMat3x2";
        case RValueOperator::ConstructDMat3x3: return "ConstructDMat3x3";
        case RValueOperator::ConstructDMat3x4: return "ConstructDMat3x4";
        case RValueOperator::ConstructDMat4x2: return "ConstructDMat4x2";
        case RValueOperator::ConstructDMat4x3: return "ConstructDMat4x3";
        case RValueOperator::ConstructDMat4x4: return "ConstructDMat4x4";
        case RValueOperator::ConstructIMat2x2: return "ConstructIMat2x2";
        case RValueOperator::ConstructIMat2x3: return "ConstructIMat2x3";
        case RValueOperator::ConstructIMat2x4: return "ConstructIMat2x4";
        case RValueOperator::ConstructIMat3x2: return "ConstructIMat3x2";
        case RValueOperator::ConstructIMat3x3: return "ConstructIMat3x3";
        case RValueOperator::ConstructIMat3x4: return "ConstructIMat3x4";
        case RValueOperator::ConstructIMat4x2: return "ConstructIMat4x2";
        case RValueOperator::ConstructIMat4x3: return "ConstructIMat4x3";
        case RValueOperator::ConstructIMat4x4: return "ConstructIMat4x4";
        case RValueOperator::ConstructUMat2x2: return "ConstructUMat2x2";
        case RValueOperator::ConstructUMat2x3: return "ConstructUMat2x3";
        case RValueOperator::ConstructUMat2x4: return "ConstructUMat2x4";
        case RValueOperator::ConstructUMat3x2: return "ConstructUMat3x2";
        case RValueOperator::ConstructUMat3x3: return "ConstructUMat3x3";
        case RValueOperator::ConstructUMat3x4: return "ConstructUMat3x4";
        case RValueOperator::ConstructUMat4x2: return "ConstructUMat4x2";
        case RValueOperator::ConstructUMat4x3: return "ConstructUMat4x3";
        case RValueOperator::ConstructUMat4x4: return "ConstructUMat4x4";
        case RValueOperator::ConstructBMat2x2: return "ConstructBMat2x2";
        case RValueOperator::ConstructBMat2x3: return "ConstructBMat2x3";
        case RValueOperator::ConstructBMat2x4: return "ConstructBMat2x4";
        case RValueOperator::ConstructBMat3x2: return "ConstructBMat3x2";
        case RValueOperator::ConstructBMat3x3: return "ConstructBMat3x3";
        case RValueOperator::ConstructBMat3x4: return "ConstructBMat3x4";
        case RValueOperator::ConstructBMat4x2: return "ConstructBMat4x2";
        case RValueOperator::ConstructBMat4x3: return "ConstructBMat4x3";
        case RValueOperator::ConstructBMat4x4: return "ConstructBMat4x4";
        case RValueOperator::ConstructFloat16: return "ConstructFloat16";
        case RValueOperator::ConstructF16Vec2: return "ConstructF16Vec2";
        case RValueOperator::ConstructF16Vec3: return "ConstructF16Vec3";
        case RValueOperator::ConstructF16Vec4: return "ConstructF16Vec4";
        case RValueOperator::ConstructF16Mat2x2: return "ConstructF16Mat2x2";
        case RValueOperator::ConstructF16Mat2x3: return "ConstructF16Mat2x3";
        case RValueOperator::ConstructF16Mat2x4: return "ConstructF16Mat2x4";
        case RValueOperator::ConstructF16Mat3x2: return "ConstructF16Mat3x2";
        case RValueOperator::ConstructF16Mat3x3: return "ConstructF16Mat3x3";
        case RValueOperator::ConstructF16Mat3x4: return "ConstructF16Mat3x4";
        case RValueOperator::ConstructF16Mat4x2: return "ConstructF16Mat4x2";
        case RValueOperator::ConstructF16Mat4x3: return "ConstructF16Mat4x3";
        case RValueOperator::ConstructF16Mat4x4: return "ConstructF16Mat4x4";
        case RValueOperator::ConstructStruct: return "ConstructStruct";
        case RValueOperator::ConstructTextureSampler: return "ConstructTextureSampler";
        case RValueOperator::ConstructNonuniform: return "ConstructNonuniform";
        case RValueOperator::ConstructReference: return "ConstructReference";
        case RValueOperator::ConstructCooperativeMatrixNV: return "ConstructCooperativeMatrixNV";
        case RValueOperator::ConstructCooperativeMatrixKHR: return "ConstructCooperativeMatrixKHR";
        case RValueOperator::ConstructAccStruct: return "ConstructAccStruct";
        case RValueOperator::ConstructGuardEnd: return "ConstructGuardEnd";
        case RValueOperator::Assign: return "Assign";
        case RValueOperator::AddAssign: return "AddAssign";
        case RValueOperator::SubAssign: return "SubAssign";
        case RValueOperator::MulAssign: return "MulAssign";
        case RValueOperator::VectorTimesMatrixAssign: return "VectorTimesMatrixAssign";
        case RValueOperator::VectorTimesScalarAssign: return "VectorTimesScalarAssign";
        case RValueOperator::MatrixTimesScalarAssign: return "MatrixTimesScalarAssign";
        case RValueOperator::MatrixTimesMatrixAssign: return "MatrixTimesMatrixAssign";
        case RValueOperator::DivAssign: return "DivAssign";
        case RValueOperator::ModAssign: return "ModAssign";
        case RValueOperator::AndAssign: return "AndAssign";
        case RValueOperator::InclusiveOrAssign: return "InclusiveOrAssign";
        case RValueOperator::ExclusiveOrAssign: return "ExclusiveOrAssign";
        case RValueOperator::LeftShiftAssign: return "LeftShiftAssign";
        case RValueOperator::RightShiftAssign: return "RightShiftAssign";
        case RValueOperator::ArrayLength: return "ArrayLength";
        case RValueOperator::ImageGuardBegin: return "ImageGuardBegin";
        case RValueOperator::ImageQuerySize: return "ImageQuerySize";
        case RValueOperator::ImageQuerySamples: return "ImageQuerySamples";
        case RValueOperator::ImageLoad: return "ImageLoad";
        case RValueOperator::ImageStore: return "ImageStore";
        case RValueOperator::ImageLoadLod: return "ImageLoadLod";
        case RValueOperator::ImageStoreLod: return "ImageStoreLod";
        case RValueOperator::ImageAtomicAdd: return "ImageAtomicAdd";
        case RValueOperator::ImageAtomicMin: return "ImageAtomicMin";
        case RValueOperator::ImageAtomicMax: return "ImageAtomicMax";
        case RValueOperator::ImageAtomicAnd: return "ImageAtomicAnd";
        case RValueOperator::ImageAtomicOr: return "ImageAtomicOr";
        case RValueOperator::ImageAtomicXor: return "ImageAtomicXor";
        case RValueOperator::ImageAtomicExchange: return "ImageAtomicExchange";
        case RValueOperator::ImageAtomicCompSwap: return "ImageAtomicCompSwap";
        case RValueOperator::ImageAtomicLoad: return "ImageAtomicLoad";
        case RValueOperator::ImageAtomicStore: return "ImageAtomicStore";
        case RValueOperator::SubpassLoad: return "SubpassLoad";
        case RValueOperator::SubpassLoadMS: return "SubpassLoadMS";
        case RValueOperator::SparseImageLoad: return "SparseImageLoad";
        case RValueOperator::SparseImageLoadLod: return "SparseImageLoadLod";
        case RValueOperator::ColorAttachmentReadEXT: return "ColorAttachmentReadEXT";
        case RValueOperator::ImageGuardEnd: return "ImageGuardEnd";
        case RValueOperator::TextureGuardBegin: return "TextureGuardBegin";
        case RValueOperator::TextureQuerySize: return "TextureQuerySize";
        case RValueOperator::TextureQueryLod: return "TextureQueryLod";
        case RValueOperator::TextureQueryLevels: return "TextureQueryLevels";
        case RValueOperator::TextureQuerySamples: return "TextureQuerySamples";
        case RValueOperator::SamplingGuardBegin: return "SamplingGuardBegin";
        case RValueOperator::Texture: return "Texture";
        case RValueOperator::TextureProj: return "TextureProj";
        case RValueOperator::TextureLod: return "TextureLod";
        case RValueOperator::TextureOffset: return "TextureOffset";
        case RValueOperator::TextureFetch: return "TextureFetch";
        case RValueOperator::TextureFetchOffset: return "TextureFetchOffset";
        case RValueOperator::TextureProjOffset: return "TextureProjOffset";
        case RValueOperator::TextureLodOffset: return "TextureLodOffset";
        case RValueOperator::TextureProjLod: return "TextureProjLod";
        case RValueOperator::TextureProjLodOffset: return "TextureProjLodOffset";
        case RValueOperator::TextureGrad: return "TextureGrad";
        case RValueOperator::TextureGradOffset: return "TextureGradOffset";
        case RValueOperator::TextureProjGrad: return "TextureProjGrad";
        case RValueOperator::TextureProjGradOffset: return "TextureProjGradOffset";
        case RValueOperator::TextureGather: return "TextureGather";
        case RValueOperator::TextureGatherOffset: return "TextureGatherOffset";
        case RValueOperator::TextureGatherOffsets: return "TextureGatherOffsets";
        case RValueOperator::TextureClamp: return "TextureClamp";
        case RValueOperator::TextureOffsetClamp: return "TextureOffsetClamp";
        case RValueOperator::TextureGradClamp: return "TextureGradClamp";
        case RValueOperator::TextureGradOffsetClamp: return "TextureGradOffsetClamp";
        case RValueOperator::TextureGatherLod: return "TextureGatherLod";
        case RValueOperator::TextureGatherLodOffset: return "TextureGatherLodOffset";
        case RValueOperator::TextureGatherLodOffsets: return "TextureGatherLodOffsets";
        case RValueOperator::FragmentMaskFetch: return "FragmentMaskFetch";
        case RValueOperator::FragmentFetch: return "FragmentFetch";
        case RValueOperator::SparseTextureGuardBegin: return "SparseTextureGuardBegin";
        case RValueOperator::SparseTexture: return "SparseTexture";
        case RValueOperator::SparseTextureLod: return "SparseTextureLod";
        case RValueOperator::SparseTextureOffset: return "SparseTextureOffset";
        case RValueOperator::SparseTextureFetch: return "SparseTextureFetch";
        case RValueOperator::SparseTextureFetchOffset: return "SparseTextureFetchOffset";
        case RValueOperator::SparseTextureLodOffset: return "SparseTextureLodOffset";
        case RValueOperator::SparseTextureGrad: return "SparseTextureGrad";
        case RValueOperator::SparseTextureGradOffset: return "SparseTextureGradOffset";
        case RValueOperator::SparseTextureGather: return "SparseTextureGather";
        case RValueOperator::SparseTextureGatherOffset: return "SparseTextureGatherOffset";
        case RValueOperator::SparseTextureGatherOffsets: return "SparseTextureGatherOffsets";
        case RValueOperator::SparseTexelsResident: return "SparseTexelsResident";
        case RValueOperator::SparseTextureClamp: return "SparseTextureClamp";
        case RValueOperator::SparseTextureOffsetClamp: return "SparseTextureOffsetClamp";
        case RValueOperator::SparseTextureGradClamp: return "SparseTextureGradClamp";
        case RValueOperator::SparseTextureGradOffsetClamp: return "SparseTextureGradOffsetClamp";
        case RValueOperator::SparseTextureGatherLod: return "SparseTextureGatherLod";
        case RValueOperator::SparseTextureGatherLodOffset: return "SparseTextureGatherLodOffset";
        case RValueOperator::SparseTextureGatherLodOffsets: return "SparseTextureGatherLodOffsets";
        case RValueOperator::SparseTextureGuardEnd: return "SparseTextureGuardEnd";
        case RValueOperator::ImageFootprintGuardBegin: return "ImageFootprintGuardBegin";
        case RValueOperator::ImageSampleFootprintNV: return "ImageSampleFootprintNV";
        case RValueOperator::ImageSampleFootprintClampNV: return "ImageSampleFootprintClampNV";
        case RValueOperator::ImageSampleFootprintLodNV: return "ImageSampleFootprintLodNV";
        case RValueOperator::ImageSampleFootprintGradNV: return "ImageSampleFootprintGradNV";
        case RValueOperator::ImageSampleFootprintGradClampNV: return "ImageSampleFootprintGradClampNV";
        case RValueOperator::ImageFootprintGuardEnd: return "ImageFootprintGuardEnd";
        case RValueOperator::SamplingGuardEnd: return "SamplingGuardEnd";
        case RValueOperator::TextureGuardEnd: return "TextureGuardEnd";
        case RValueOperator::AddCarry: return "AddCarry";
        case RValueOperator::SubBorrow: return "SubBorrow";
        case RValueOperator::UMulExtended: return "UMulExtended";
        case RValueOperator::IMulExtended: return "IMulExtended";
        case RValueOperator::BitfieldExtract: return "BitfieldExtract";
        case RValueOperator::BitfieldInsert: return "BitfieldInsert";
        case RValueOperator::BitFieldReverse: return "BitFieldReverse";
        case RValueOperator::BitCount: return "BitCount";
        case RValueOperator::FindLSB: return "FindLSB";
        case RValueOperator::FindMSB: return "FindMSB";
        case RValueOperator::CountLeadingZeros: return "CountLeadingZeros";
        case RValueOperator::CountTrailingZeros: return "CountTrailingZeros";
        case RValueOperator::AbsDifference: return "AbsDifference";
        case RValueOperator::AddSaturate: return "AddSaturate";
        case RValueOperator::SubSaturate: return "SubSaturate";
        case RValueOperator::Average: return "Average";
        case RValueOperator::AverageRounded: return "AverageRounded";
        case RValueOperator::Mul32x16: return "Mul32x16";
        case RValueOperator::TraceNV: return "TraceNV";
        case RValueOperator::TraceRayMotionNV: return "TraceRayMotionNV";
        case RValueOperator::TraceKHR: return "TraceKHR";
        case RValueOperator::ReportIntersection: return "ReportIntersection";
        case RValueOperator::IgnoreIntersectionNV: return "IgnoreIntersectionNV";
        case RValueOperator::TerminateRayNV: return "TerminateRayNV";
        case RValueOperator::ExecuteCallableNV: return "ExecuteCallableNV";
        case RValueOperator::ExecuteCallableKHR: return "ExecuteCallableKHR";
        case RValueOperator::WritePackedPrimitiveIndices4x8NV: return "WritePackedPrimitiveIndices4x8NV";
        case RValueOperator::EmitMeshTasksEXT: return "EmitMeshTasksEXT";
        case RValueOperator::SetMeshOutputsEXT: return "SetMeshOutputsEXT";
        case RValueOperator::RayQueryInitialize: return "RayQueryInitialize";
        case RValueOperator::RayQueryTerminate: return "RayQueryTerminate";
        case RValueOperator::RayQueryGenerateIntersection: return "RayQueryGenerateIntersection";
        case RValueOperator::RayQueryConfirmIntersection: return "RayQueryConfirmIntersection";
        case RValueOperator::RayQueryProceed: return "RayQueryProceed";
        case RValueOperator::RayQueryGetIntersectionType: return "RayQueryGetIntersectionType";
        case RValueOperator::RayQueryGetRayTMin: return "RayQueryGetRayTMin";
        case RValueOperator::RayQueryGetRayFlags: return "RayQueryGetRayFlags";
        case RValueOperator::RayQueryGetIntersectionT: return "RayQueryGetIntersectionT";
        case RValueOperator::RayQueryGetIntersectionInstanceCustomIndex: return "RayQueryGetIntersectionInstanceCustomIndex";
        case RValueOperator::RayQueryGetIntersectionInstanceId: return "RayQueryGetIntersectionInstanceId";
        case RValueOperator::RayQueryGetIntersectionInstanceShaderBindingTableRecordOffset: return "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffset";
        case RValueOperator::RayQueryGetIntersectionGeometryIndex: return "RayQueryGetIntersectionGeometryIndex";
        case RValueOperator::RayQueryGetIntersectionPrimitiveIndex: return "RayQueryGetIntersectionPrimitiveIndex";
        case RValueOperator::RayQueryGetIntersectionBarycentrics: return "RayQueryGetIntersectionBarycentrics";
        case RValueOperator::RayQueryGetIntersectionFrontFace: return "RayQueryGetIntersectionFrontFace";
        case RValueOperator::RayQueryGetIntersectionCandidateAABBOpaque: return "RayQueryGetIntersectionCandidateAABBOpaque";
        case RValueOperator::RayQueryGetIntersectionObjectRayDirection: return "RayQueryGetIntersectionObjectRayDirection";
        case RValueOperator::RayQueryGetIntersectionObjectRayOrigin: return "RayQueryGetIntersectionObjectRayOrigin";
        case RValueOperator::RayQueryGetWorldRayDirection: return "RayQueryGetWorldRayDirection";
        case RValueOperator::RayQueryGetWorldRayOrigin: return "RayQueryGetWorldRayOrigin";
        case RValueOperator::RayQueryGetIntersectionObjectToWorld: return "RayQueryGetIntersectionObjectToWorld";
        case RValueOperator::RayQueryGetIntersectionWorldToObject: return "RayQueryGetIntersectionWorldToObject";
        case RValueOperator::HitObjectTraceRayNV: return "HitObjectTraceRayNV";
        case RValueOperator::HitObjectTraceRayMotionNV: return "HitObjectTraceRayMotionNV";
        case RValueOperator::HitObjectRecordHitNV: return "HitObjectRecordHitNV";
        case RValueOperator::HitObjectRecordHitMotionNV: return "HitObjectRecordHitMotionNV";
        case RValueOperator::HitObjectRecordHitWithIndexNV: return "HitObjectRecordHitWithIndexNV";
        case RValueOperator::HitObjectRecordHitWithIndexMotionNV: return "HitObjectRecordHitWithIndexMotionNV";
        case RValueOperator::HitObjectRecordMissNV: return "HitObjectRecordMissNV";
        case RValueOperator::HitObjectRecordMissMotionNV: return "HitObjectRecordMissMotionNV";
        case RValueOperator::HitObjectRecordEmptyNV: return "HitObjectRecordEmptyNV";
        case RValueOperator::HitObjectExecuteShaderNV: return "HitObjectExecuteShaderNV";
        case RValueOperator::HitObjectIsEmptyNV: return "HitObjectIsEmptyNV";
        case RValueOperator::HitObjectIsMissNV: return "HitObjectIsMissNV";
        case RValueOperator::HitObjectIsHitNV: return "HitObjectIsHitNV";
        case RValueOperator::HitObjectGetRayTMinNV: return "HitObjectGetRayTMinNV";
        case RValueOperator::HitObjectGetRayTMaxNV: return "HitObjectGetRayTMaxNV";
        case RValueOperator::HitObjectGetObjectRayOriginNV: return "HitObjectGetObjectRayOriginNV";
        case RValueOperator::HitObjectGetObjectRayDirectionNV: return "HitObjectGetObjectRayDirectionNV";
        case RValueOperator::HitObjectGetWorldRayOriginNV: return "HitObjectGetWorldRayOriginNV";
        case RValueOperator::HitObjectGetWorldRayDirectionNV: return "HitObjectGetWorldRayDirectionNV";
        case RValueOperator::HitObjectGetWorldToObjectNV: return "HitObjectGetWorldToObjectNV";
        case RValueOperator::HitObjectGetObjectToWorldNV: return "HitObjectGetObjectToWorldNV";
        case RValueOperator::HitObjectGetInstanceCustomIndexNV: return "HitObjectGetInstanceCustomIndexNV";
        case RValueOperator::HitObjectGetInstanceIdNV: return "HitObjectGetInstanceIdNV";
        case RValueOperator::HitObjectGetGeometryIndexNV: return "HitObjectGetGeometryIndexNV";
        case RValueOperator::HitObjectGetPrimitiveIndexNV: return "HitObjectGetPrimitiveIndexNV";
        case RValueOperator::HitObjectGetHitKindNV: return "HitObjectGetHitKindNV";
        case RValueOperator::HitObjectGetShaderBindingTableRecordIndexNV: return "HitObjectGetShaderBindingTableRecordIndexNV";
        case RValueOperator::HitObjectGetShaderRecordBufferHandleNV: return "HitObjectGetShaderRecordBufferHandleNV";
        case RValueOperator::HitObjectGetAttributesNV: return "HitObjectGetAttributesNV";
        case RValueOperator::HitObjectGetCurrentTimeNV: return "HitObjectGetCurrentTimeNV";
        case RValueOperator::ReorderThreadNV: return "ReorderThreadNV";
        case RValueOperator::FetchMicroTriangleVertexPositionNV: return "FetchMicroTriangleVertexPositionNV";
        case RValueOperator::FetchMicroTriangleVertexBarycentricNV: return "FetchMicroTriangleVertexBarycentricNV";
    }
}

BranchOperator glslangOperatorToBranchOperator(glslang::TOperator op) {
    switch (op) {
        case EOpKill: return BranchOperator::Discard;
        case EOpTerminateInvocation: return BranchOperator::TerminateInvocation;
        case EOpDemote: return BranchOperator::Demote;
        case EOpTerminateRayKHR: return BranchOperator::TerminateRayEXT;
        case EOpIgnoreIntersectionKHR: return BranchOperator::IgnoreIntersectionEXT;
        case EOpReturn: return BranchOperator::Return;
        case EOpBreak: return BranchOperator::Break;
        case EOpContinue: return BranchOperator::Continue;
        case EOpCase: return BranchOperator::Case;
        case EOpDefault: return BranchOperator::Default;
        default:
            utils::slog.e << "Cannot convert operator " << operatorToString(op) << " to Branch operator.";
            // TODO: abort here
            return BranchOperator::Discard;
    }
}

template<typename Id, typename Value>
class IdStore {
public:
    // Throws if non-existent.
    Value getById(Id id) {
        return mIdToValue.at(id);
    }

    // Inserts if non-existent.
    Id getOrInsertByValue(Value value) {
        auto it = mValueToId.find(value);
        if (it == mValueToId.end()) {
            Id id = Id {mIdToValue.size() + 1};
            mIdToValue[id] = value;
            mValueToId[value] = id;
            return id;
        } else {
            return it->second;
        }
    }

private:
    std::unordered_map<Id, Value> mIdToValue;
    std::unordered_map<Value, Id> mValueToId;
};

class Slurper {
public:
    void slurpFromRoot(glslang::TIntermAggregate* node) {
        assert(node != nullptr);
        assert(node->getOp() == glslang::EOpSequence);

        for (TIntermNode* child : node->getSequence()) {
            if (auto childAsAggregate = child->getAsAggregate()) {
                switch (childAsAggregate->getOp()) {
                    case glslang::EOpFunction:
                        mFunctionDefinitions.push_back(slurpFunctionDefinition(childAsAggregate, node));
                        continue;
                    case glslang::EOpLinkerObjects:
                        // TODO: linker objects
                        continue;
                    case glslang::EOpSequence:
                        // TODO: Why does this appear?
                        continue;
                    default:
                        // Fall through.
                        break;
                }
            }
            utils::slog.e << "Unhandled child of root node: "
                          << nodeToString(child) << " " << locToString(child->getLoc())
                          << utils::io::endl;
        }
    }

    void dumpAll(std::stringstream &out) {
        for (const auto& functionDefinition : mFunctionDefinitions) {
            auto name = mFunctionNames.getById(functionDefinition.name);
            out << "RTYPE " << name << "PARAMS)";
            if (functionDefinition.body) {
                out << " {\n";
                dump(functionDefinition.body.value(), 1, out);
                out << "}\n";
            } else {
                out << ";\n";
            }
        }
    }

private:
    IdStore<TypeId, Type> mTypes;
    std::unordered_map<TypeId, IdStore<LValueId, LValue>> mLValues;
    std::unordered_map<TypeId, IdStore<RValueId, RValue>> mRValues;
    IdStore<FunctionId, std::string_view> mFunctionNames;
    IdStore<StatementBlockId, std::vector<Statement>> mStatementBlocks;
    std::vector<FunctionDefinition> mFunctionDefinitions;

    StatementBlockId slurpStatementBlock(TIntermNode* node, TIntermNode* parent) {
        std::vector<Statement> statements;
        auto nodeAsAggregate = node->getAsAggregate();
        if (nodeAsAggregate != nullptr && nodeAsAggregate->getOp() == glslang::EOpSequence) {
            // Read all children into this statement block.
            for (TIntermNode* child : nodeAsAggregate->getSequence()) {
                if (parent->getAsSwitchNode()) {
                    utils::slog.i << "child of switch: " << nodeToString(child) << utils::io::endl;
                }
                nodeToStatements(child, node, statements);
            }
        } else {
            // Wrap whatever this is into a new statement block.
            nodeToStatements(node, parent, statements);
        }
        return mStatementBlocks.getOrInsertByValue(statements);
    }

    FunctionDefinition slurpFunctionDefinition(
            glslang::TIntermAggregate* node, TIntermNode* parent) {
        assert(node->getOp() == glslang::EOpFunction);
        auto& sequence = node->getSequence();
        assert(sequence.size() == 1 || sequence.size() == 2);
        auto parameters = sequence[0]->getAsAggregate();
        assert(parameters != nullptr);

        auto functionId = mFunctionNames.getOrInsertByValue(node->getName());
        std::optional<StatementBlockId> bodyId;
        if (sequence.size() == 2) {
            bodyId = slurpStatementBlock(sequence[1], node);
        } else {
            // Prototype. No body ID.
        }
        return FunctionDefinition{functionId, bodyId};
    }

    // Turn a non-root node into one or more statements.
    void nodeToStatements(TIntermNode* node, TIntermNode* parent, std::vector<Statement> &output) {
        if (auto nodeAsLoopNode = node->getAsLoopNode()) {
            auto conditionId = slurpValue(nodeAsLoopNode->getTest(), parent);
            std::optional<RValueId> terminalId;
            if (nodeAsLoopNode->getTerminal()) {
                auto terminalIdAsValueId = slurpValue(nodeAsLoopNode->getTerminal(), parent);
                if (auto* terminalIdAsLValueId = std::get_if<LValueId>(&terminalIdAsValueId)) {
                    // Ignore random stray LValues, since they don't do anything.
                } else if (auto* terminalIdAsRValueId = std::get_if<RValueId>(&terminalIdAsValueId)) {
                    terminalId = *terminalIdAsRValueId;
                } else {
                    assert(false);
                }
            }
            bool testFirst = nodeAsLoopNode->testFirst();
            auto bodyId = slurpStatementBlock(nodeAsLoopNode->getBody(), parent);
            output.push_back(LoopStatement{conditionId, terminalId, testFirst, bodyId});
            return;
        }
        if (auto nodeAsBranchNode = node->getAsBranchNode()) {
            auto op = glslangOperatorToBranchOperator(nodeAsBranchNode->getFlowOp());
            std::optional<ValueId> operandId;
            if (auto operand = nodeAsBranchNode->getExpression()) {
                operandId = slurpValue(operand, node);
            }
            output.push_back(BranchStatement{op, operandId});
            return;
        }
        if (auto nodeAsSwitchNode = node->getAsSwitchNode()) {
            if (auto conditionAsTyped = nodeAsSwitchNode->getCondition()->getAsTyped()) {
                auto conditionId = slurpValue(conditionAsTyped, parent);
                auto bodyId = slurpStatementBlock(nodeAsSwitchNode->getBody(), parent);
                output.push_back(SwitchStatement{conditionId, bodyId});
            } else {
                utils::slog.e << "Switch node condition was not typed: "
                              << nodeToString(nodeAsSwitchNode->getCondition())
                              << ", parent: " << nodeToString(parent)
                              << " " << locToString(node->getLoc()) << utils::io::endl;
            }
            return;
        }
        if (auto nodeAsSelectionNode = node->getAsSelectionNode()) {
            auto conditionId = slurpValue(nodeAsSelectionNode->getCondition(), parent);
            auto trueId = slurpStatementBlock(nodeAsSelectionNode->getTrueBlock(), parent);
            std::optional<StatementBlockId> falseId;
            if (nodeAsSelectionNode->getFalseBlock()) {
                falseId = slurpStatementBlock(nodeAsSelectionNode->getFalseBlock(), parent);
            }
            output.push_back(IfStatement{conditionId, trueId, falseId});
            return;
        }
        if (auto nodeAsAggregate = node->getAsAggregate()) {
            switch (nodeAsAggregate->getOp()) {
                case glslang::EOpSequence:
                    // Flatten this.
                    for (auto child : nodeAsAggregate->getSequence()) {
                        nodeToStatements(child, node, output);
                    }
                    return;
                default:
                    // Fall through and interpret the node as a value instead of an expression.
                    break;
            }
        }
        if (auto nodeAsTyped = node->getAsTyped()) {
            auto valueId = slurpValue(nodeAsTyped, parent);
            if (auto* lValueId = std::get_if<LValueId>(&valueId)) {
                // Ignore random stray LValues, since they don't do anything.
            } else if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
                output.push_back(*rValueId);
            } else {
                assert(false);
            }
            return;
        }
        utils::slog.e << "Cannot convert to statement: "
                      << nodeToString(node)
                      << ", parent: " << nodeToString(parent)
                      << " " << locToString(node->getLoc()) << utils::io::endl;
    }

    ValueId slurpValue(TIntermTyped* node, TIntermNode* parent) {
        auto typeId = slurpType(node->getType());
        if (auto nodeAsSymbol = node->getAsSymbolNode()) {
            return mLValues[typeId].getOrInsertByValue(LValue{nodeAsSymbol->getName()});
        }
        if (auto nodeAsConstantUnion = node->getAsConstantUnion()) {
            return mRValues[typeId].getOrInsertByValue(LiteralRValue{});
        }
        if (auto nodeAsUnary = node->getAsUnaryNode()) {
            auto operand = slurpValue(nodeAsUnary->getOperand(), node);
            return mRValues[typeId].getOrInsertByValue(OperatorRValue{glslangOperatorToRValueOperator(nodeAsUnary->getOp()), {operand}});
        }
        if (auto nodeAsBinary = node->getAsBinaryNode()) {
            switch (nodeAsBinary->getOp()) {
                case glslang::EOpVectorSwizzle: {
                    // TODO: swizzle it up
                    auto swizzle = nodeAsBinary->getRight()->getAsAggregate();
                    assert(swizzle != nullptr);
                    assert(swizzle->getOp() == glslang::EOpSequence);
                    return mRValues[typeId].getOrInsertByValue(OperatorRValue{RValueOperator::VectorSwizzle});
                }
                default: {
                    auto lhsId = slurpValue(nodeAsBinary->getLeft(), node);
                    auto rhsId = slurpValue(nodeAsBinary->getRight(), node);
                    return mRValues[typeId].getOrInsertByValue(OperatorRValue{glslangOperatorToRValueOperator(nodeAsBinary->getOp()), {lhsId, rhsId}});
                }
            }
        }
        if (auto nodeAsSelection = node->getAsSelectionNode()) {
            // A "selection" as interpreted as an expression is a ternary.
            auto conditionId = slurpValue(nodeAsSelection->getCondition(), parent);
            auto trueNodeAsTyped = nodeAsSelection->getTrueBlock()->getAsTyped();
            auto falseNodeAsTyped = nodeAsSelection->getFalseBlock()->getAsTyped();
            if (trueNodeAsTyped && falseNodeAsTyped) {
                auto trueId = slurpValue(trueNodeAsTyped, parent);
                auto falseId = slurpValue(falseNodeAsTyped, parent);
                return mRValues[typeId].getOrInsertByValue(OperatorRValue{RValueOperator::Ternary, {conditionId, trueId, falseId}});
            } else {
                utils::slog.e << "A selection node branch was not typed: "
                              << "true = " << nodeToString(nodeAsSelection->getTrueBlock())
                              << ", false = " << nodeToString(nodeAsSelection->getFalseBlock())
                              << ", parent: " << nodeToString(parent)
                              << " " << locToString(node->getLoc()) << utils::io::endl;
                // TODO: assert here
                return RValueId{};
            }
        }
        if (auto nodeAsAggregate = node->getAsAggregate()) {
            auto& sequence = nodeAsAggregate->getSequence();
            switch (nodeAsAggregate->getOp()) {
                case glslang::EOpFunction:
                case glslang::EOpLinkerObjects:
                case glslang::EOpParameters:
                case glslang::EOpSequence:
                    // Explicitly ban these from becoming RValues, since we probably made a mistake
                    // somewhere...
                    break;
                case glslang::EOpFunctionCall: {
                    auto functionId = mFunctionNames.getOrInsertByValue(nodeAsAggregate->getName());
                    std::vector<ValueId> args;
                    for (TIntermNode* arg : sequence) {
                        if (auto argAsTyped = arg->getAsTyped()) {
                            args.push_back(slurpValue(argAsTyped, node));
                        } else {
                            utils::slog.e << "Function call argument was not typed: "
                                          << nodeToString(arg)
                                          << ", parent: " << nodeToString(parent)
                                          << " " << locToString(node->getLoc()) << utils::io::endl;
                            // TODO: assert here
                        }
                    }
                    return mRValues[typeId].getOrInsertByValue(
                            FunctionCallRValue{functionId, std::move(args)});
                }
                default: {
                    std::vector<ValueId> args;
                    for (TIntermNode* arg : sequence) {
                        if (auto argAsTyped = arg->getAsTyped()) {
                            args.push_back(slurpValue(argAsTyped, node));
                        } else {
                            utils::slog.e << "Operator argument was not typed: "
                                          << nodeToString(arg)
                                          << ", parent: " << nodeToString(parent)
                                          << " " << locToString(node->getLoc()) << utils::io::endl;
                            // TODO: assert here
                        }
                    }
                    return mRValues[typeId].getOrInsertByValue(
                            OperatorRValue{glslangOperatorToRValueOperator(nodeAsAggregate->getOp()), std::move(args)});
                }
            }
        }
        utils::slog.e << "Cannot convert to value: " << nodeToString(node)
                      << ", parent: " << nodeToString(parent)
                      << " " << locToString(node->getLoc()) << utils::io::endl;
        // TODO: assert here
        return RValueId{};
    }

    TypeId slurpType(const TType& type) {
        auto typeArraySizes = type.getArraySizes();
        std::vector<std::size_t> arraySizes(typeArraySizes ? typeArraySizes->getNumDims() : 0);
        for (int i = 0; i < arraySizes.size(); ++i) {
            arraySizes[i] = typeArraySizes->getDimSize(i);
        }
        return mTypes.getOrInsertByValue(Type{
            BuiltInType::Void,
            std::max(type.getMatrixCols(), 1),
            std::max(type.getMatrixRows(), type.getVectorSize()),
            std::move(arraySizes)
        });
    }

    void dump(StatementBlockId blockId, int depth, std::stringstream &out) {
        std::string indentMinusOne;
        for (int i = 0; i < depth - 1; ++i) {
            indentMinusOne += "  ";
        }
        std::string indent = indentMinusOne;
        if (depth > 0) {
            indent += "  ";
        }
        for (auto statement : mStatementBlocks.getById(blockId)) {
            if (auto* rValueId = std::get_if<RValueId>(&statement)) {
                out << indent;
                dumpRValue(*rValueId, out);
                out << ";\n";
            } else if (auto* ifStatement = std::get_if<IfStatement>(&statement)) {
                out << indent << "if (";
                dumpValue(ifStatement->condition, out);
                out << ") {\n";
                dump(ifStatement->thenBlock, depth + 1, out);
                if (ifStatement->elseBlock) {
                    out << indent << "} else {\n";
                    dump(ifStatement->elseBlock.value(), depth + 1, out);
                }
                out << indent << "}\n";
            } else if (auto* switchStatement = std::get_if<SwitchStatement>(&statement)) {
                out << indent << "switch (";
                dumpValue(switchStatement->condition, out);
                out << ") {\n";
                dump(switchStatement->body, depth + 1, out);
                out << indent << "}\n";
            } else if (auto* branchStatement = std::get_if<BranchStatement>(&statement)) {
                switch (branchStatement->op) {
                    case BranchOperator::Discard:
                        out << indent << "discard";
                        break;
                    case BranchOperator::TerminateInvocation:
                        out << indent << "terminateInvocation";
                        break;
                    case BranchOperator::Demote:
                        out << indent << "demote";
                        break;
                    case BranchOperator::TerminateRayEXT:
                        out << indent << "terminateRayEXT";
                        break;
                    case BranchOperator::IgnoreIntersectionEXT:
                        out << indent << "terminateIntersectionEXT";
                        break;
                    case BranchOperator::Return:
                        out << indent << "return";
                        break;
                    case BranchOperator::Break:
                        out << indent << "break";
                        break;
                    case BranchOperator::Continue:
                        out << indent << "continue";
                        break;
                    case BranchOperator::Case:
                        out << indentMinusOne << "case";
                        break;
                    case BranchOperator::Default:
                        out << indentMinusOne << "default";
                        break;
                }
                if (branchStatement->operand) {
                    out << " ";
                    dumpValue(branchStatement->operand.value(), out);
                }
                switch (branchStatement->op) {
                    case BranchOperator::Case:
                    case BranchOperator::Default:
                        out << ":\n";
                        break;
                    default:
                        out << ";\n";
                        break;
                }
            } else if (auto* loopStatement = std::get_if<LoopStatement>(&statement)) {
                if (loopStatement->testFirst) {
                    if (loopStatement->terminal) {
                        out << indent << "for (; ";
                        dumpValue(loopStatement->condition, out);
                        out << "; ";
                        dumpRValue(loopStatement->terminal.value(), out);
                    } else {
                        out << indent << "while (";
                        dumpValue(loopStatement->condition, out);
                    }
                    out << ") {\n";
                    dump(loopStatement->body, depth + 1, out);
                    out << indent << "}\n";
                } else {
                    out << indent << "do {\n";
                    dump(loopStatement->body, depth + 1, out);
                    out << indent << "} while (";
                    dumpValue(loopStatement->condition, out);
                    out << ");\n";
                }
            } else {
                assert(false);
            }
        }
    }

    void dumpValue(TypeId typeId, ValueId valueId, std::stringstream &out) {
        if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
            dumpRValue(typeId, *rValueId, out);
        } else if (auto *lValueId = std::get_if<LValueId>(&valueId)) {
            dumpLValue(typeId, *lValueId, out);
        } else {
            assert(false);
        }
    }

    void dumpRValue(TypeId typeId, RValueId rValueId, std::stringstream &out) {
        if (rValueId.id == 0) {
            out << "INVALID_RVALUE";
            return;
        }
        auto rValue = mRValues.at(typeId).getById(rValueId);
        if (auto* functionCall = std::get_if<FunctionCallRValue>(&rValue)) {
            auto name = mFunctionNames.getById(functionCall->function);
            out << "(" << name;
            for (auto& arg : functionCall->args) {
                out << " ";
                dumpValue(arg, out);
            }
            out << ")";
        } else if (auto* op = std::get_if<OperatorRValue>(&rValue)) {
            out << "(" << rValueOperatorToString(op->op);
            for (auto& arg : op->args) {
                out << " ";
                dumpValue(arg, out);
            }
            out << ")";
        } else if (auto* literal = std::get_if<LiteralRValue>(&rValue)) {
            out << "LITERAL";
        } else {
            assert(false);
        }
    }

    void dumpLValue(TypeId typeId, LValueId lValueId, std::stringstream &out) {
        if (lValueId.id == 0) {
            out << "INVALID_LVALUE";
            return;
        }
        auto lValue = mLValues.at(typeId).getById(lValueId);
        out << lValue.name;
    }
};

void Komi::slurp(const glslang::TIntermediate& intermediate) {
    Slurper slurper;
    slurper.slurpFromRoot(intermediate.getTreeRoot()->getAsAggregate());
    std::stringstream glsl;
    slurper.dumpAll(glsl);
    utils::slog.i << std::move(glsl.str()) << utils::io::endl;
}

} // namespace glslkomi
