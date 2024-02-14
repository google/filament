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
#include <glslkomi/Debug.h>
#include <intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <unordered_map>
#include "utils/Log.h"
#include <sstream>
#include <utils/Panic.h>

namespace glslkomi {

RValueOperator glslangOperatorToRValueOperator(glslang::TOperator op) {
    using namespace glslang;
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
            utils::slog.e << "Cannot convert operator " << glslangOperatorToString(op) << " to RValue operator.";
            // TODO: abort here
            return RValueOperator::Ternary;
    }
}

BranchOperator glslangOperatorToBranchOperator(glslang::TOperator op) {
    using namespace glslang;
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
            PANIC_PRECONDITION("Cannot convert operator %s to BranchOperator",
                    glslangOperatorToString(op));
    }
}

std::string_view functionSignatureToName(std::string_view functionSignature) {
    auto indexParenthesis = functionSignature.find('(');
    return functionSignature.substr(0, indexParenthesis);
}

std::string_view expandTypeNameToVector(
        const char* const* typeNames, int vectorSize) {
    ASSERT_PRECONDITION(vectorSize >= 1 && vectorSize <= 4,
            "vectorSize must be between 1 and 4");
    return typeNames[vectorSize - 1];
}

std::string_view expandTypeNameToVectorOrMatrix(
        const char* const* typeNames, bool isMatrix,
        int vectorSize, int matrixCols, int matrixRows) {
    if (isMatrix) {
        ASSERT_PRECONDITION(matrixCols >= 2 && matrixCols <= 4,
                "matrixCols must be between 2 and 4");
        ASSERT_PRECONDITION(matrixRows >= 2 && matrixRows <= 4,
                "matrixRows must be between 2 and 4");
        return typeNames[
                4 // 4 to skip vector
                + (matrixCols - 2) * 3
                + (matrixRows - 2)
                ];
    }
    return expandTypeNameToVector(typeNames, vectorSize);
}

Type glslangTypeToType(const glslang::TType& type) {
    using namespace glslang;

    static const char* const FLOAT_TYPE_NAMES[] = {
            "float",
            "vec2",
            "vec3",
            "vec4",
            "mat2",
            "mat2x3",
            "mat2x4",
            "mat3x2",
            "mat3",
            "mat3x4",
            "mat4x2",
            "mat4x3",
            "mat4",
    };

    static const char* const DOUBLE_TYPE_NAMES[] = {
            "double",
            "dvec2",
            "dvec3",
            "dvec4",
            "dmat2",
            "dmat2x3",
            "dmat2x4",
            "dmat3x2",
            "dmat3",
            "dmat3x4",
            "dmat4x2",
            "dmat4x3",
            "dmat4",
    };

    static const char* const INT_TYPE_NAMES[] = {
            "int",
            "ivec2",
            "ivec3",
            "ivec4",
    };

    static const char* const UINT_TYPE_NAMES[] = {
            "uint",
            "uvec2",
            "uvec3",
            "uvec4",
    };

    static const char* const BOOL_TYPE_NAMES[] = {
            "bool",
            "bvec2",
            "bvec3",
            "bvec4",
    };

    auto typeArraySizes = type.getArraySizes();
    std::vector<std::size_t> arraySizes(typeArraySizes ? typeArraySizes->getNumDims() : 0);
    for (int i = 0; i < arraySizes.size(); ++i) {
        arraySizes[i] = typeArraySizes->getDimSize(i);
    }

    std::string_view typeName;
    switch (type.getBasicType()) {
        case EbtVoid:
            typeName = "void";
            break;
        case EbtFloat:
            typeName = expandTypeNameToVectorOrMatrix(
                    FLOAT_TYPE_NAMES,
                    type.isMatrix(),
                    type.getVectorSize(),
                    type.getMatrixCols(),
                    type.getMatrixRows());
            break;
        case EbtDouble:
            typeName = expandTypeNameToVectorOrMatrix(
                    DOUBLE_TYPE_NAMES,
                    type.isMatrix(),
                    type.getVectorSize(),
                    type.getMatrixCols(),
                    type.getMatrixRows());
            break;
        case EbtInt:
            typeName = expandTypeNameToVector(
                    INT_TYPE_NAMES,
                    type.getVectorSize());
            break;
        case EbtUint:
            typeName = expandTypeNameToVector(
                    UINT_TYPE_NAMES,
                    type.getVectorSize());
            break;
        case EbtBool:
            typeName = expandTypeNameToVector(
                    BOOL_TYPE_NAMES,
                    type.getVectorSize());
            break;
        case EbtAtomicUint:
            typeName = "atomic_uint";
            break;
        case EbtSampler:
            typeName = type.getSampler().getString();
            break;
        case EbtStruct:
        case EbtBlock:
            typeName = type.getTypeName();
            break;
        default:
            PANIC_PRECONDITION("Cannot convert glslang type `%s' to Type",
                    type.getCompleteString().c_str());
    };
    return Type{typeName, type.getPrecisionQualifierString(), std::move(arraySizes)};
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
        ASSERT_PRECONDITION(node != nullptr, "Node must not be null");
        ASSERT_PRECONDITION(node->getOp() == glslang::EOpSequence, "Node must be a sequence");

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
            PANIC_PRECONDITION("Unhandled child of root node: %s, parent = %s",
                    glslangNodeToStringWithLoc(child).c_str(),
                    glslangNodeToStringWithLoc(node).c_str());
        }
    }

    void dumpAll(std::stringstream &out) {
        for (const auto& functionDefinition : mFunctionDefinitions) {
            auto name = mFunctionNames.getById(functionDefinition.name);
            dumpType(functionDefinition.returnType, out);
            out << " " << name << "(";
            bool firstParameter = true;
            for (const auto& parameter : functionDefinition.parameters) {
                if (firstParameter) {
                    firstParameter = false;
                } else {
                    out << ", ";
                }
                dumpType(parameter.type, out);
                out << " ";
                dumpLValue(parameter.name, out);
            }
            out << ")";
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
    IdStore<LValueId, LValue> mLValues;
    IdStore<RValueId, RValue> mRValues;
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
                    utils::slog.i << "child of switch: " << glslangNodeToString(child)
                                  << utils::io::endl;
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
        ASSERT_PRECONDITION(node->getOp() == glslang::EOpFunction, "Node must be a function");
        auto& sequence = node->getSequence();
        ASSERT_PRECONDITION(sequence.size() == 1 || sequence.size() == 2, "Sequence must be of length 1 or 2");
        auto parametersNode = sequence[0]->getAsAggregate();
        ASSERT_PRECONDITION(parametersNode != nullptr, "Function parameters must be an aggregate node");

        auto functionId = mFunctionNames.getOrInsertByValue(
            functionSignatureToName(node->getName()));
        auto returnTypeId =
            mTypes.getOrInsertByValue(glslangTypeToType(node->getType()));

        std::vector<FunctionParameter> parameters;
        for (const auto parameter : parametersNode->getSequence()) {
            auto parameterAsSymbol = parameter->getAsSymbolNode();
            ASSERT_PRECONDITION(parameterAsSymbol != nullptr,
                    "Function parameter must be symbol: %s, definition = %s, parent = %s",
                    glslangNodeToStringWithLoc(parameter).c_str(),
                    glslangNodeToStringWithLoc(node).c_str(),
                    glslangNodeToStringWithLoc(parent).c_str());
            auto nameId = mLValues.getOrInsertByValue(LValue{parameterAsSymbol->getName()});
            auto typeId = mTypes.getOrInsertByValue(glslangTypeToType(parameterAsSymbol->getType()));
            parameters.push_back(FunctionParameter{nameId, typeId});
        }

        std::optional<StatementBlockId> bodyId;
        if (sequence.size() == 2) {
            bodyId = slurpStatementBlock(sequence[1], node);
        } else {
            // Prototype. No body ID.
        }
        return FunctionDefinition{functionId, returnTypeId, std::move(parameters), bodyId};
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
                    PANIC_PRECONDITION("unreachable");
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
                PANIC_PRECONDITION("Switch node condition was not typed: %s, parent = %s",
                        glslangNodeToStringWithLoc(nodeAsSwitchNode->getCondition()).c_str(),
                        glslangNodeToString(parent).c_str());
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
                PANIC_PRECONDITION("Unreachable");
            }
            return;
        }
        PANIC_PRECONDITION("Cannot convert to statement: %s, parent = %s",
                glslangNodeToStringWithLoc(node).c_str(),
                glslangNodeToStringWithLoc(parent).c_str());
    }

    ValueId slurpValue(glslang::TIntermTyped* node, TIntermNode* parent) {
        auto typeId = mTypes.getOrInsertByValue(glslangTypeToType(node->getType()));
        if (auto nodeAsSymbol = node->getAsSymbolNode()) {
            return mLValues.getOrInsertByValue(LValue{nodeAsSymbol->getName()});
        }
        if (auto nodeAsConstantUnion = node->getAsConstantUnion()) {
            return mRValues.getOrInsertByValue(LiteralRValue{});
        }
        if (auto nodeAsUnary = node->getAsUnaryNode()) {
            auto operand = slurpValue(nodeAsUnary->getOperand(), node);
            return mRValues.getOrInsertByValue(OperatorRValue{glslangOperatorToRValueOperator(nodeAsUnary->getOp()), {operand}});
        }
        if (auto nodeAsBinary = node->getAsBinaryNode()) {
            switch (nodeAsBinary->getOp()) {
                case glslang::EOpVectorSwizzle: {
                    // TODO: swizzle it up
                    auto swizzle = nodeAsBinary->getRight()->getAsAggregate();
                    ASSERT_PRECONDITION(swizzle != nullptr, "Swizzle node must be an aggregate");
                    ASSERT_PRECONDITION(swizzle->getOp() == glslang::EOpSequence, "Swizzle node must be a sequence");
                    return mRValues.getOrInsertByValue(OperatorRValue{RValueOperator::VectorSwizzle});
                }
                default: {
                    auto lhsId = slurpValue(nodeAsBinary->getLeft(), node);
                    auto rhsId = slurpValue(nodeAsBinary->getRight(), node);
                    return mRValues.getOrInsertByValue(OperatorRValue{glslangOperatorToRValueOperator(nodeAsBinary->getOp()), {lhsId, rhsId}});
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
                return mRValues.getOrInsertByValue(OperatorRValue{RValueOperator::Ternary, {conditionId, trueId, falseId}});
            } else {
                PANIC_PRECONDITION("A selection node branch was not typed: true = %s, false = %s, parent = %s %s",
                        glslangNodeToStringWithLoc(nodeAsSelection->getTrueBlock()).c_str(),
                        glslangNodeToStringWithLoc(nodeAsSelection->getFalseBlock()).c_str(),
                        glslangNodeToStringWithLoc(parent).c_str());
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
                    auto functionId = mFunctionNames.getOrInsertByValue(functionSignatureToName(nodeAsAggregate->getName()));
                    std::vector<ValueId> args;
                    for (TIntermNode* arg : sequence) {
                        if (auto argAsTyped = arg->getAsTyped()) {
                            args.push_back(slurpValue(argAsTyped, node));
                        } else {
                            PANIC_PRECONDITION("Function call argument was not typed: arg = %s, function = %s, parent = %s %s",
                                    glslangNodeToStringWithLoc(arg).c_str(),
                                    glslangNodeToStringWithLoc(node).c_str(),
                                    glslangNodeToStringWithLoc(parent).c_str());
                        }
                    }
                    return mRValues.getOrInsertByValue(
                            FunctionCallRValue{functionId, std::move(args)});
                }
                default: {
                    std::vector<ValueId> args;
                    for (TIntermNode* arg : sequence) {
                        if (auto argAsTyped = arg->getAsTyped()) {
                            args.push_back(slurpValue(argAsTyped, node));
                        } else {
                            PANIC_PRECONDITION("Operator argument was not typed: arg = %s, function = %s, parent = %s %s",
                                    glslangNodeToStringWithLoc(arg).c_str(),
                                    glslangNodeToStringWithLoc(node).c_str(),
                                    glslangNodeToStringWithLoc(parent).c_str());
                        }
                    }
                    return mRValues.getOrInsertByValue(
                            OperatorRValue{glslangOperatorToRValueOperator(nodeAsAggregate->getOp()), std::move(args)});
                }
            }
        }
        PANIC_PRECONDITION("Cannot convert to statement: %s, parent = %s",
                glslangNodeToStringWithLoc(node).c_str(),
                glslangNodeToStringWithLoc(parent).c_str());
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
                PANIC_PRECONDITION("Unreachable");
            }
        }
    }

    void dumpValue(ValueId valueId, std::stringstream &out) {
        if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
            dumpRValue(*rValueId, out);
        } else if (auto *lValueId = std::get_if<LValueId>(&valueId)) {
            dumpLValue(*lValueId, out);
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
    }

    void dumpRValue(RValueId rValueId, std::stringstream &out) {
        if (rValueId.id == 0) {
            out << "INVALID_RVALUE";
            return;
        }
        auto rValue = mRValues.getById(rValueId);
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
            PANIC_PRECONDITION("Unreachable");
        }
    }

    void dumpLValue(LValueId lValueId, std::stringstream &out) {
        if (lValueId.id == 0) {
            out << "INVALID_LVALUE";
            return;
        }
        auto lValue = mLValues.getById(lValueId);
        out << lValue.name;
    }

    void dumpType(TypeId typeId, std::stringstream &out) {
        auto type = mTypes.getById(typeId);
        if (!type.precision.empty()) {
            out << type.precision << " ";
        }
        out << type.name;
        for (const auto& arraySize : type.arraySizes) {
            out << "[" << arraySize << "]";
        }
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
