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

std::variant<RValueOperator, std::string_view> glslangOperatorToRValueOperator(
        glslang::TOperator op, int version, Type returnType, std::optional<Type> arg1Type) {
    using namespace glslang;
    switch (op) {
        case EOpNegative: return RValueOperator::Negative;
        case EOpLogicalNot: return RValueOperator::LogicalNot;
        case EOpVectorLogicalNot: return "not";
        case EOpBitwiseNot: return RValueOperator::BitwiseNot;
        case EOpPostIncrement: return RValueOperator::PostIncrement;
        case EOpPostDecrement: return RValueOperator::PostDecrement;
        case EOpPreIncrement: return RValueOperator::PreIncrement;
        case EOpPreDecrement: return RValueOperator::PreDecrement;
        case EOpConvIntToBool:
        case EOpConvUintToBool:
        case EOpConvFloatToBool:
        case EOpConvDoubleToBool:
            return "bool";
        case EOpConvBoolToInt:
        case EOpConvUintToInt:
        case EOpConvFloatToInt:
        case EOpConvDoubleToInt:
            return "int";
        case EOpConvBoolToFloat:
        case EOpConvIntToFloat:
        case EOpConvUintToFloat:
        case EOpConvDoubleToFloat:
            return "float";
        case EOpConvBoolToDouble:
        case EOpConvIntToDouble:
        case EOpConvUintToDouble:
        case EOpConvFloatToDouble:
            return "double";
        case EOpConvBoolToUint:
        case EOpConvIntToUint:
        case EOpConvFloatToUint:
        case EOpConvDoubleToUint:
            return "uint";
        case EOpAdd: return RValueOperator::Add;
        case EOpSub: return RValueOperator::Sub;
        case EOpMul:
        case EOpVectorTimesScalar:
        case EOpVectorTimesMatrix:
        case EOpMatrixTimesVector:
        case EOpMatrixTimesScalar:
            return RValueOperator::Mul;
        case EOpDiv: return RValueOperator::Div;
        case EOpMod: return RValueOperator::Mod;
        case EOpRightShift: return RValueOperator::RightShift;
        case EOpLeftShift: return RValueOperator::LeftShift;
        case EOpAnd: return RValueOperator::And;
        case EOpInclusiveOr: return RValueOperator::InclusiveOr;
        case EOpExclusiveOr: return RValueOperator::ExclusiveOr;
        case EOpEqual: return RValueOperator::Equal;
        case EOpNotEqual: return RValueOperator::NotEqual;
        case EOpVectorEqual: return "equal";
        case EOpVectorNotEqual: return "notEqual";
        case EOpLessThan: return RValueOperator::LessThan;
        case EOpGreaterThan: return RValueOperator::GreaterThan;
        case EOpLessThanEqual: return RValueOperator::LessThanEqual;
        case EOpGreaterThanEqual: return RValueOperator::GreaterThanEqual;
        case EOpComma: return RValueOperator::Comma;
        case EOpLogicalOr: return RValueOperator::LogicalOr;
        case EOpLogicalXor: return RValueOperator::LogicalXor;
        case EOpLogicalAnd: return RValueOperator::LogicalAnd;
        case EOpIndexDirect: return RValueOperator::IndexDirect;
        case EOpIndexIndirect: return RValueOperator::IndexIndirect;
        case EOpIndexDirectStruct: return RValueOperator::IndexDirectStruct;
        case EOpVectorSwizzle: return RValueOperator::VectorSwizzle;
        case EOpRadians: return "radians";
        case EOpDegrees: return "degrees";
        case EOpSin: return "sin";
        case EOpCos: return "cos";
        case EOpTan: return "tan";
        case EOpAsin: return "asin";
        case EOpAcos: return "acos";
        case EOpAtan: return "atan";
        case EOpSinh: return "sinh";
        case EOpCosh: return "cosh";
        case EOpTanh: return "tanh";
        case EOpAsinh: return "asinh";
        case EOpAcosh: return "acosh";
        case EOpAtanh: return "atanh";
        case EOpPow: return "pow";
        case EOpExp: return "exp";
        case EOpLog: return "log";
        case EOpExp2: return "exp2";
        case EOpLog2: return "log2";
        case EOpSqrt: return "sqrt";
        case EOpInverseSqrt: return "inversesqrt";
        case EOpAbs: return "abs";
        case EOpSign: return "sign";
        case EOpFloor: return "floor";
        case EOpTrunc: return "trunc";
        case EOpRound: return "round";
        case EOpRoundEven: return "roundEven";
        case EOpCeil: return "ceil";
        case EOpFract: return "fract";
        case EOpModf: return "modf";
        case EOpMin: return "min";
        case EOpMax: return "max";
        case EOpClamp: return "clamp";
        case EOpMix: return "mix";
        case EOpStep: return "step";
        case EOpSmoothStep: return "smoothstep";
        case EOpIsNan: return "isnan";
        case EOpIsInf: return "isinf";
        case EOpFma: return "fma";
        case EOpFrexp: return "frexp";
        case EOpLdexp: return "ldexp";
        case EOpFloatBitsToInt: return "floatBitsToInt";
        case EOpFloatBitsToUint: return "floatBitsToUint";
        case EOpIntBitsToFloat: return "intBitsToFloat";
        case EOpUintBitsToFloat: return "uintBitsToFloat";
        case EOpPackSnorm2x16: return "packSnorm2x16";
        case EOpUnpackSnorm2x16: return "unpackSnorm2x16";
        case EOpPackUnorm2x16: return "packUnorm2x16";
        case EOpUnpackUnorm2x16: return "unpackUnorm2x16";
        case EOpPackSnorm4x8: return "packSnorm4x8";
        case EOpUnpackSnorm4x8: return "unpackSnorm4x8";
        case EOpPackUnorm4x8: return "packUnorm4x8";
        case EOpUnpackUnorm4x8: return "unpackUnorm4x8";
        case EOpPackHalf2x16: return "packHalf2x16";
        case EOpUnpackHalf2x16: return "unpackHalf2x16";
        case EOpPackDouble2x32: return "packDouble2x32";
        case EOpUnpackDouble2x32: return "unpackDouble2x32";
        case EOpPackInt2x32: return "packInt2x32";
        case EOpUnpackInt2x32: return "unpackInt2x32";
        case EOpPackUint2x32: return "packUint2x32";
        case EOpUnpackUint2x32: return "unpackUint2x32";
        case EOpPackFloat2x16: return "packFloat2x16";
        case EOpUnpackFloat2x16: return "unpackFloat2x16";
        case EOpPackInt2x16: return "packInt2x16";
        case EOpUnpackInt2x16: return "unpackInt2x16";
        case EOpPackUint2x16: return "packUint2x16";
        case EOpUnpackUint2x16: return "unpackUint2x16";
        case EOpPackInt4x16: return "packInt4x16";
        case EOpUnpackInt4x16: return "unpackInt4x16";
        case EOpPackUint4x16: return "packUint4x16";
        case EOpUnpackUint4x16: return "unpackUint4x16";
        case EOpPack16: return "pack16";
        case EOpPack32: return "pack32";
        case EOpPack64: return "pack64";
        case EOpUnpack32: return "unpack32";
        case EOpUnpack16: return "unpack16";
        case EOpUnpack8: return "unpack8";
        case EOpLength: return "length";
        case EOpDistance: return "distance";
        case EOpDot: return "dot";
        case EOpCross: return "cross";
        case EOpNormalize: return "normalize";
        case EOpFaceForward: return "faceforward";
        case EOpReflect: return "reflect";
        case EOpRefract: return "refract";
        case EOpMin3: return "min3";
        case EOpMax3: return "max3";
        case EOpMid3: return "mid3";
        case EOpDPdx: return "dFdx";
        case EOpDPdy: return "dFdy";
        case EOpFwidth: return "fwidth";
        case EOpDPdxFine: return "dFdxFine";
        case EOpDPdyFine: return "dFdyFine";
        case EOpFwidthFine: return "fwidthFine";
        case EOpDPdxCoarse: return "dFdxCoarse";
        case EOpDPdyCoarse: return "dFdyCoarse";
        case EOpFwidthCoarse: return "fwidthCoarse";
        case EOpInterpolateAtCentroid: return "interpolateAtCentroid";
        case EOpInterpolateAtSample: return "interpolateAtSample";
        case EOpInterpolateAtOffset: return "interpolateAtOffset";
        case EOpInterpolateAtVertex: return "interpolateAtVertexAMD";
        case EOpOuterProduct: return "outerProduct";
        case EOpDeterminant: return "determinant";
        case EOpMatrixInverse: return "inverse";
        case EOpTranspose: return "transpose";
        case EOpFtransform: return "ftransform";
        case EOpEmitVertex: return "EmitVertex";
        case EOpEndPrimitive: return "EndPrimitive";
        case EOpEmitStreamVertex: return "EmitStreamVertex";
        case EOpEndStreamPrimitive: return "EndStreamPrimitive";
        case EOpBarrier: return "barrier";
        case EOpMemoryBarrier: return "memoryBarrier";
        case EOpMemoryBarrierAtomicCounter: return "memoryBarrierAtomicCounter";
        case EOpMemoryBarrierBuffer: return "memoryBarrierBuffer";
        case EOpMemoryBarrierImage: return "memoryBarrierImage";
        case EOpMemoryBarrierShared: return "memoryBarrierShared";
        case EOpGroupMemoryBarrier: return "groupMemoryBarrier";
        case EOpBallot: return "ballotARB";
        case EOpReadInvocation: return "readInvocationARB";
        case EOpReadFirstInvocation: return "readFirstInvocationARB";
        case EOpAnyInvocation: return version >= 460 ? "anyInvocation" : "anyInvocationARB";
        case EOpAllInvocations: return version >= 460 ? "allInvocations" : "allInvocationsARB";
        case EOpAllInvocationsEqual: return version >= 460 ? "allInvocationsEqual" : "allInvocationsEqualARB";
        case EOpSubgroupBarrier: return "subgroupBarrier";
        case EOpSubgroupMemoryBarrier: return "subgroupMemoryBarrier";
        case EOpSubgroupMemoryBarrierBuffer: return "subgroupMemoryBarrierBuffer";
        case EOpSubgroupMemoryBarrierImage: return "subgroupMemoryBarrierImage";
        case EOpSubgroupMemoryBarrierShared: return "subgroupMemoryBarrierShared";
        case EOpSubgroupElect: return "subgroupElect";
        case EOpSubgroupAll: return "subgroupAll";
        case EOpSubgroupAny: return "subgroupAny";
        case EOpSubgroupAllEqual: return "subgroupAllEqual";
        case EOpSubgroupBroadcast: return "subgroupBroadcast";
        case EOpSubgroupBroadcastFirst: return "subgroupBroadcastFirst";
        case EOpSubgroupBallot: return "subgroupBallot";
        case EOpSubgroupInverseBallot: return "subgroupInverseBallot";
        case EOpSubgroupBallotBitExtract: return "subgroupBallotBitExtract";
        case EOpSubgroupBallotBitCount: return "subgroupBallotBitCount";
        case EOpSubgroupBallotInclusiveBitCount: return "subgroupBallotInclusiveBitCount";
        case EOpSubgroupBallotExclusiveBitCount: return "subgroupBallotExclusiveBitCount";
        case EOpSubgroupBallotFindLSB: return "subgroupBallotFindLSB";
        case EOpSubgroupBallotFindMSB: return "subgroupBallotFindMSB";
        case EOpSubgroupShuffle: return "subgroupShuffle";
        case EOpSubgroupShuffleXor: return "subgroupShuffleXor";
        case EOpSubgroupShuffleUp: return "subgroupShuffleUp";
        case EOpSubgroupShuffleDown: return "subgroupShuffleDown";
        case EOpSubgroupAdd: return "subgroupAdd";
        case EOpSubgroupMul: return "subgroupMul";
        case EOpSubgroupMin: return "subgroupMin";
        case EOpSubgroupMax: return "subgroupMax";
        case EOpSubgroupAnd: return "subgroupAnd";
        case EOpSubgroupOr: return "subgroupOr";
        case EOpSubgroupXor: return "subgroupXor";
        case EOpSubgroupInclusiveAdd: return "subgroupInclusiveAdd";
        case EOpSubgroupInclusiveMul: return "subgroupInclusiveMul";
        case EOpSubgroupInclusiveMin: return "subgroupInclusiveMin";
        case EOpSubgroupInclusiveMax: return "subgroupInclusiveMax";
        case EOpSubgroupInclusiveAnd: return "subgroupInclusiveAnd";
        case EOpSubgroupInclusiveOr: return "subgroupInclusiveOr";
        case EOpSubgroupInclusiveXor: return "subgroupInclusiveXor";
        case EOpSubgroupExclusiveAdd: return "subgroupExclusiveAdd";
        case EOpSubgroupExclusiveMul: return "subgroupExclusiveMul";
        case EOpSubgroupExclusiveMin: return "subgroupExclusiveMin";
        case EOpSubgroupExclusiveMax: return "subgroupExclusiveMax";
        case EOpSubgroupExclusiveAnd: return "subgroupExclusiveAnd";
        case EOpSubgroupExclusiveOr: return "subgroupExclusiveOr";
        case EOpSubgroupExclusiveXor: return "subgroupExclusiveXor";
        case EOpSubgroupClusteredAdd: return "subgroupClusteredAdd";
        case EOpSubgroupClusteredMul: return "subgroupClusteredMul";
        case EOpSubgroupClusteredMin: return "subgroupClusteredMin";
        case EOpSubgroupClusteredMax: return "subgroupClusteredMax";
        case EOpSubgroupClusteredAnd: return "subgroupClusteredAnd";
        case EOpSubgroupClusteredOr: return "subgroupClusteredOr";
        case EOpSubgroupClusteredXor: return "subgroupClusteredXor";
        case EOpSubgroupQuadBroadcast: return "subgroupQuadBroadcast";
        case EOpSubgroupQuadSwapHorizontal: return "subgroupQuadSwapHorizontal";
        case EOpSubgroupQuadSwapVertical: return "subgroupQuadSwapVertical";
        case EOpSubgroupQuadSwapDiagonal: return "subgroupQuadSwapDiagonal";
        case EOpSubgroupPartition: return "subgroupPartitionNV";
        case EOpSubgroupPartitionedAdd: return "subgroupPartitionedAddNV";
        case EOpSubgroupPartitionedMul: return "subgroupPartitionedMulNV";
        case EOpSubgroupPartitionedMin: return "subgroupPartitionedMinNV";
        case EOpSubgroupPartitionedMax: return "subgroupPartitionedMaxNV";
        case EOpSubgroupPartitionedAnd: return "subgroupPartitionedAndNV";
        case EOpSubgroupPartitionedOr: return "subgroupPartitionedOrNV";
        case EOpSubgroupPartitionedXor: return "subgroupPartitionedXorNV";
        case EOpSubgroupPartitionedInclusiveAdd: return "subgroupPartitionedInclusiveAddNV";
        case EOpSubgroupPartitionedInclusiveMul: return "subgroupPartitionedInclusiveMulNV";
        case EOpSubgroupPartitionedInclusiveMin: return "subgroupPartitionedInclusiveMinNV";
        case EOpSubgroupPartitionedInclusiveMax: return "subgroupPartitionedInclusiveMaxNV";
        case EOpSubgroupPartitionedInclusiveAnd: return "subgroupPartitionedInclusiveAndNV";
        case EOpSubgroupPartitionedInclusiveOr: return "subgroupPartitionedInclusiveOrNV";
        case EOpSubgroupPartitionedInclusiveXor: return "subgroupPartitionedInclusiveXorNV";
        case EOpSubgroupPartitionedExclusiveAdd: return "subgroupPartitionedExclusiveAddNV";
        case EOpSubgroupPartitionedExclusiveMul: return "subgroupPartitionedExclusiveMulNV";
        case EOpSubgroupPartitionedExclusiveMin: return "subgroupPartitionedExclusiveMinNV";
        case EOpSubgroupPartitionedExclusiveMax: return "subgroupPartitionedExclusiveMaxNV";
        case EOpSubgroupPartitionedExclusiveAnd: return "subgroupPartitionedExclusiveAndNV";
        case EOpSubgroupPartitionedExclusiveOr: return "subgroupPartitionedExclusiveOrNV";
        case EOpSubgroupPartitionedExclusiveXor: return "subgroupPartitionedExclusiveXorNV";
        case EOpMinInvocations: return "minInvocationsAMD";
        case EOpMaxInvocations: return "maxInvocationsAMD";
        case EOpAddInvocations: return "addInvocationsAMD";
        case EOpMinInvocationsNonUniform: return "minInvocationsNonUniformAMD";
        case EOpMaxInvocationsNonUniform: return "maxInvocationsNonUniformAMD";
        case EOpAddInvocationsNonUniform: return "addInvocationsNonUniformAMD";
        case EOpMinInvocationsInclusiveScan: return "minInvocationsInclusiveScanAMD";
        case EOpMaxInvocationsInclusiveScan: return "maxInvocationsInclusiveScanAMD";
        case EOpAddInvocationsInclusiveScan: return "addInvocationsInclusiveScanAMD";
        case EOpMinInvocationsInclusiveScanNonUniform: return "minInvocationsInclusiveScanNonUniformAMD";
        case EOpMaxInvocationsInclusiveScanNonUniform: return "maxInvocationsInclusiveScanNonUniformAMD";
        case EOpAddInvocationsInclusiveScanNonUniform: return "addInvocationsInclusiveScanNonUniformAMD";
        case EOpMinInvocationsExclusiveScan: return "minInvocationsExclusiveScanAMD";
        case EOpMaxInvocationsExclusiveScan: return "maxInvocationsExclusiveScanAMD";
        case EOpAddInvocationsExclusiveScan: return "addInvocationsExclusiveScanAMD";
        case EOpMinInvocationsExclusiveScanNonUniform: return "minInvocationsExclusiveScanNonUniformAMD";
        case EOpMaxInvocationsExclusiveScanNonUniform: return "maxInvocationsExclusiveScanNonUniformAMD";
        case EOpAddInvocationsExclusiveScanNonUniform: return "addInvocationsExclusiveScanNonUniformAMD";
        case EOpSwizzleInvocations: return "swizzleInvocationsAMD";
        case EOpSwizzleInvocationsMasked: return "swizzleInvocationsMaskedAMD";
        case EOpWriteInvocation: return "writeInvocationAMD";
        case EOpMbcnt: return "mbcntAMD";
        case EOpCubeFaceIndex: return "cubeFaceIndexAMD";
        case EOpCubeFaceCoord: return "cubeFaceCoordAMD";
        case EOpTime: return "timeAMD";
        case EOpAtomicAdd: return "atomicAdd";
        case EOpAtomicMin: return "atomicMin";
        case EOpAtomicMax: return "atomicMax";
        case EOpAtomicAnd: return "atomicAnd";
        case EOpAtomicOr: return "atomicOr";
        case EOpAtomicXor: return "atomicXor";
        case EOpAtomicExchange: return "atomicExchange";
        case EOpAtomicCompSwap: return "atomicCompSwap";
        case EOpAtomicLoad: return "atomicLoad";
        case EOpAtomicStore: return "atomicStore";
        case EOpAtomicCounterIncrement: return "atomicCounterIncrement";
        case EOpAtomicCounterDecrement: return "atomicCounterDecrement";
        case EOpAtomicCounter: return "atomicCounter";
        case EOpAtomicCounterAdd: return version >= 460 ? "atomicCounterAdd" : "atomicCounterAddARB";
        case EOpAtomicCounterSubtract: return version >= 460 ? "atomicCounterSubtract" : "atomicCounterSubtractARB";
        case EOpAtomicCounterMin: return version >= 460 ? "atomicCounterMin" : "atomicCounterMinARB";
        case EOpAtomicCounterMax: return version >= 460 ? "atomicCounterMax" : "atomicCounterMaxARB";
        case EOpAtomicCounterAnd: return version >= 460 ? "atomicCounterAnd" : "atomicCounterAndARB";
        case EOpAtomicCounterOr: return version >= 460 ? "atomicCounterOr" : "atomicCounterOrARB";
        case EOpAtomicCounterXor: return version >= 460 ? "atomicCounterXor" : "atomicCounterXorARB";
        case EOpAtomicCounterExchange: return version >= 460 ? "atomicCounterExchange" : "atomicCounterExchangeARB";
        case EOpAtomicCounterCompSwap: return version >= 460 ? "atomicCounterCompSwap" : "atomicCounterCompSwapARB";
        case EOpAny: return "any";
        case EOpAll: return "all";
        case EOpCooperativeMatrixLoad: return "coopMatLoad";
        case EOpCooperativeMatrixStore: return "coopMatStore";
        case EOpCooperativeMatrixMulAdd: return "coopMatMulAdd";
        case EOpCooperativeMatrixLoadNV: return "coopMatLoadNV";
        case EOpCooperativeMatrixStoreNV: return "coopMatStoreNV";
        case EOpCooperativeMatrixMulAddNV: return "coopMatMulAddNV";
        case EOpBeginInvocationInterlock: return "beginInvocationInterlockARB";
        case EOpEndInvocationInterlock: return "endInvocationInterlockARB";
        case EOpIsHelperInvocation: return "helperInvocationEXT";
        case EOpDebugPrintf: return "debugPrintfEXT";
        case EOpConstructInt: return "int";
        case EOpConstructUint: return "uint";
        case EOpConstructInt8: return "int8";
        case EOpConstructUint8: return "uint8";
        case EOpConstructInt16: return "int16";
        case EOpConstructUint16: return "uint16";
        case EOpConstructInt64: return "int64";
        case EOpConstructUint64: return "uint64";
        case EOpConstructBool: return "bool";
        case EOpConstructFloat: return "float";
        case EOpConstructDouble: return "double";
        case EOpConstructVec2: return "vec2";
        case EOpConstructVec3: return "vec3";
        case EOpConstructVec4: return "vec4";
        case EOpConstructMat2x2: return "mat2x2";
        case EOpConstructMat2x3: return "mat2x3";
        case EOpConstructMat2x4: return "mat2x4";
        case EOpConstructMat3x2: return "mat3x2";
        case EOpConstructMat3x3: return "mat3x3";
        case EOpConstructMat3x4: return "mat3x4";
        case EOpConstructMat4x2: return "mat4x2";
        case EOpConstructMat4x3: return "mat4x3";
        case EOpConstructMat4x4: return "mat4x4";
        case EOpConstructDVec2: return "dvec2";
        case EOpConstructDVec3: return "dvec3";
        case EOpConstructDVec4: return "dvec4";
        case EOpConstructBVec2: return "bvec2";
        case EOpConstructBVec3: return "bvec3";
        case EOpConstructBVec4: return "bvec4";
        case EOpConstructI8Vec2: return "i8vec2";
        case EOpConstructI8Vec3: return "i8vec3";
        case EOpConstructI8Vec4: return "i8vec4";
        case EOpConstructU8Vec2: return "u8vec2";
        case EOpConstructU8Vec3: return "u8vec3";
        case EOpConstructU8Vec4: return "u8vec4";
        case EOpConstructI16Vec2: return "i16vec2";
        case EOpConstructI16Vec3: return "i16vec3";
        case EOpConstructI16Vec4: return "i16vec4";
        case EOpConstructU16Vec2: return "u16vec2";
        case EOpConstructU16Vec3: return "u16vec3";
        case EOpConstructU16Vec4: return "u16vec4";
        case EOpConstructIVec2: return "ivec2";
        case EOpConstructIVec3: return "ivec3";
        case EOpConstructIVec4: return "ivec4";
        case EOpConstructUVec2: return "uvec2";
        case EOpConstructUVec3: return "uvec3";
        case EOpConstructUVec4: return "uvec4";
        case EOpConstructI64Vec2: return "i64vec2";
        case EOpConstructI64Vec3: return "i64vec3";
        case EOpConstructI64Vec4: return "i64vec4";
        case EOpConstructU64Vec2: return "u64vec2";
        case EOpConstructU64Vec3: return "u64vec3";
        case EOpConstructU64Vec4: return "u64vec4";
        case EOpConstructDMat2x2: return "dmat2x2";
        case EOpConstructDMat2x3: return "dmat2x3";
        case EOpConstructDMat2x4: return "dmat2x4";
        case EOpConstructDMat3x2: return "dmat3x2";
        case EOpConstructDMat3x3: return "dmat3x3";
        case EOpConstructDMat3x4: return "dmat3x4";
        case EOpConstructDMat4x2: return "dmat4x2";
        case EOpConstructDMat4x3: return "dmat4x3";
        case EOpConstructDMat4x4: return "dmat4x4";
        case EOpConstructIMat2x2: return "imat2x2";
        case EOpConstructIMat2x3: return "imat2x3";
        case EOpConstructIMat2x4: return "imat2x4";
        case EOpConstructIMat3x2: return "imat3x2";
        case EOpConstructIMat3x3: return "imat3x3";
        case EOpConstructIMat3x4: return "imat3x4";
        case EOpConstructIMat4x2: return "imat4x2";
        case EOpConstructIMat4x3: return "imat4x3";
        case EOpConstructIMat4x4: return "imat4x4";
        case EOpConstructUMat2x2: return "umat2x2";
        case EOpConstructUMat2x3: return "umat2x3";
        case EOpConstructUMat2x4: return "umat2x4";
        case EOpConstructUMat3x2: return "umat3x2";
        case EOpConstructUMat3x3: return "umat3x3";
        case EOpConstructUMat3x4: return "umat3x4";
        case EOpConstructUMat4x2: return "umat4x2";
        case EOpConstructUMat4x3: return "umat4x3";
        case EOpConstructUMat4x4: return "umat4x4";
        case EOpConstructBMat2x2: return "bmat2x2";
        case EOpConstructBMat2x3: return "bmat2x3";
        case EOpConstructBMat2x4: return "bmat2x4";
        case EOpConstructBMat3x2: return "bmat3x2";
        case EOpConstructBMat3x3: return "bmat3x3";
        case EOpConstructBMat3x4: return "bmat3x4";
        case EOpConstructBMat4x2: return "bmat4x2";
        case EOpConstructBMat4x3: return "bmat4x3";
        case EOpConstructBMat4x4: return "bmat4x4";
        case EOpConstructFloat16: return "float16";
        case EOpConstructF16Vec2: return "f16vec2";
        case EOpConstructF16Vec3: return "f16vec3";
        case EOpConstructF16Vec4: return "f16vec4";
        case EOpConstructF16Mat2x2: return "f16mat2x2";
        case EOpConstructF16Mat2x3: return "f16mat2x3";
        case EOpConstructF16Mat2x4: return "f16mat2x4";
        case EOpConstructF16Mat3x2: return "f16mat3x2";
        case EOpConstructF16Mat3x3: return "f16mat3x3";
        case EOpConstructF16Mat3x4: return "f16mat3x4";
        case EOpConstructF16Mat4x2: return "f16mat4x2";
        case EOpConstructF16Mat4x3: return "f16mat4x3";
        case EOpConstructF16Mat4x4: return "f16mat4x4";
        case EOpConstructStruct: return RValueOperator::ConstructStruct;
        case EOpConstructTextureSampler: return "textureSampler";
        case EOpConstructNonuniform: return "nonuniform";
        case EOpConstructReference: return "reference";
        case EOpConstructCooperativeMatrixNV: return "cooperativeMatrixNV";
        case EOpConstructCooperativeMatrixKHR: return "cooperativeMatrixKHR";
        case EOpAssign: return RValueOperator::Assign;
        case EOpAddAssign: return RValueOperator::AddAssign;
        case EOpSubAssign: return RValueOperator::SubAssign;
        case EOpMulAssign:
        case EOpVectorTimesMatrixAssign:
        case EOpVectorTimesScalarAssign:
        case EOpMatrixTimesScalarAssign:
        case EOpMatrixTimesMatrixAssign:
            return RValueOperator::MulAssign;
        case EOpDivAssign: return RValueOperator::DivAssign;
        case EOpModAssign: return RValueOperator::ModAssign;
        case EOpAndAssign: return RValueOperator::AndAssign;
        case EOpInclusiveOrAssign: return RValueOperator::InclusiveOrAssign;
        case EOpExclusiveOrAssign: return RValueOperator::ExclusiveOrAssign;
        case EOpLeftShiftAssign: return RValueOperator::LeftShiftAssign;
        case EOpRightShiftAssign: return RValueOperator::RightShiftAssign;
        case EOpArrayLength: return RValueOperator::ArrayLength;
        case EOpImageQuerySize: return "imageSize";
        case EOpImageQuerySamples: return "imageSamples";
        case EOpImageLoad: return "imageLoad";
        case EOpImageStore: return "imageStore";
        case EOpImageLoadLod: return "imageLoadLodAMD";
        case EOpImageStoreLod: return "imageStoreLodAMD";
        case EOpImageAtomicAdd: return "imageAtomicAdd";
        case EOpImageAtomicMin: return "imageAtomicMin";
        case EOpImageAtomicMax: return "imageAtomicMax";
        case EOpImageAtomicAnd: return "imageAtomicAnd";
        case EOpImageAtomicOr: return "imageAtomicOr";
        case EOpImageAtomicXor: return "imageAtomicXor";
        case EOpImageAtomicExchange: return "imageAtomicExchange";
        case EOpImageAtomicCompSwap: return "imageAtomicCompSwap";
        case EOpImageAtomicLoad: return "imageAtomicLoad";
        case EOpImageAtomicStore: return "imageAtomicStore";
        case EOpSubpassLoad: return "subpassLoad";
        case EOpSubpassLoadMS: return "subpassLoadMS";
        case EOpSparseImageLoad: return "sparseImageLoadARB";
        case EOpSparseImageLoadLod: return "sparseImageLoadLodAMD";
        case EOpColorAttachmentReadEXT: return "colorAttachmentReadEXT";
        case EOpTextureQuerySize: return "textureSize";
        case EOpTextureQueryLod: return version >= 400 ? "textureQueryLod" : "textureQueryLOD";
        case EOpTextureQueryLevels: return "textureQueryLevels";
        case EOpTextureQuerySamples: return "textureSamples";
        // case EOpTexture: return "shadow1D";
        // case EOpTexture: return "shadow2D";
        // case EOpTexture: return "shadow2DEXT";
        // case EOpTexture: return "shadow2DRect";
        // case EOpTexture: return "texture";
        // case EOpTexture: return "texture1D";
        // case EOpTexture: return "texture2D";
        // case EOpTexture: return "texture2DRect";
        // case EOpTexture: return "texture3D";
        // case EOpTexture: return "textureCube";
        // case EOpTextureProj: return "shadow1DProj";
        // case EOpTextureProj: return "shadow2DProj";
        // case EOpTextureProj: return "shadow2DProjEXT";
        // case EOpTextureProj: return "shadow2DRectProj";
        // case EOpTextureProj: return "texture1DProj";
        // case EOpTextureProj: return "texture2DProj";
        // case EOpTextureProj: return "texture2DRectProj";
        // case EOpTextureProj: return "texture3DProj";
        // case EOpTextureProj: return "textureProj";
        // case EOpTextureLod: return "shadow1DLod";
        // case EOpTextureLod: return "shadow2DLod";
        // case EOpTextureLod: return "texture1DLod";
        // case EOpTextureLod: return "texture2DLod";
        // case EOpTextureLod: return "texture2DLodEXT";
        // case EOpTextureLod: return "texture3DLod";
        // case EOpTextureLod: return "textureCubeLod";
        // case EOpTextureLod: return "textureCubeLodEXT";
        // case EOpTextureLod: return "textureLod";
        case EOpTextureOffset: return "textureOffset";
        case EOpTextureFetch: return "texelFetch";
        case EOpTextureFetchOffset: return "texelFetchOffset";
        case EOpTextureProjOffset: return "textureProjOffset";
        case EOpTextureLodOffset: return "textureLodOffset";
        // case EOpTextureProjLod: return "shadow1DProjLod";
        // case EOpTextureProjLod: return "shadow2DProjLod";
        // case EOpTextureProjLod: return "texture1DProjLod";
        // case EOpTextureProjLod: return "texture2DProjLod";
        // case EOpTextureProjLod: return "texture2DProjLodEXT";
        // case EOpTextureProjLod: return "texture3DProjLod";
        // case EOpTextureProjLod: return "textureProjLod";
        case EOpTextureProjLodOffset: return "textureProjLodOffset";
        // case EOpTextureGrad: return "shadow1DGradARB";
        // case EOpTextureGrad: return "shadow2DGradARB";
        // case EOpTextureGrad: return "shadow2DRectGradARB";
        // case EOpTextureGrad: return "texture1DGradARB";
        // case EOpTextureGrad: return "texture2DGradARB";
        // case EOpTextureGrad: return "texture2DGradEXT";
        // case EOpTextureGrad: return "texture2DRectGradARB";
        // case EOpTextureGrad: return "texture3DGradARB";
        // case EOpTextureGrad: return "textureCubeGradARB";
        // case EOpTextureGrad: return "textureCubeGradEXT";
        // case EOpTextureGrad: return "textureGrad";
        case EOpTextureGradOffset: return "textureGradOffset";
        // case EOpTextureProjGrad: return "shadow1DProjGradARB";
        // case EOpTextureProjGrad: return "shadow2DProjGradARB";
        // case EOpTextureProjGrad: return "shadow2DRectProjGradARB";
        // case EOpTextureProjGrad: return "texture1DProjGradARB";
        // case EOpTextureProjGrad: return "texture2DProjGradARB";
        // case EOpTextureProjGrad: return "texture2DProjGradEXT";
        // case EOpTextureProjGrad: return "texture2DRectProjGradARB";
        // case EOpTextureProjGrad: return "texture3DProjGradARB";
        // case EOpTextureProjGrad: return "textureProjGrad";
        case EOpTextureProjGradOffset: return "textureProjGradOffset";
        case EOpTextureGather: return "textureGather";
        case EOpTextureGatherOffset: return "textureGatherOffset";
        case EOpTextureGatherOffsets: return "textureGatherOffsets";
        case EOpTextureClamp: return "textureClampARB";
        case EOpTextureOffsetClamp: return "textureOffsetClampARB";
        case EOpTextureGradClamp: return "textureGradClampARB";
        case EOpTextureGradOffsetClamp: return "textureGradOffsetClampARB";
        case EOpTextureGatherLod: return "textureGatherLodAMD";
        case EOpTextureGatherLodOffset: return "textureGatherLodOffsetAMD";
        case EOpTextureGatherLodOffsets: return "textureGatherLodOffsetsAMD";
        case EOpFragmentMaskFetch: return "fragmentMaskFetchAMD";
        case EOpFragmentFetch: return "fragmentFetchAMD";
        case EOpSparseTexture: return "sparseTextureARB";
        case EOpSparseTextureLod: return "sparseTextureLodARB";
        case EOpSparseTextureOffset: return "sparseTextureOffsetARB";
        case EOpSparseTextureFetch: return "sparseTexelFetchARB";
        case EOpSparseTextureFetchOffset: return "sparseTexelFetchOffsetARB";
        case EOpSparseTextureLodOffset: return "sparseTextureLodOffsetARB";
        case EOpSparseTextureGrad: return "sparseTextureGradARB";
        case EOpSparseTextureGradOffset: return "sparseTextureGradOffsetARB";
        case EOpSparseTextureGather: return "sparseTextureGatherARB";
        case EOpSparseTextureGatherOffset: return "sparseTextureGatherOffsetARB";
        case EOpSparseTextureGatherOffsets: return "sparseTextureGatherOffsetsARB";
        case EOpSparseTexelsResident: return "sparseTexelsResidentARB";
        case EOpSparseTextureClamp: return "sparseTextureClampARB";
        case EOpSparseTextureOffsetClamp: return "sparseTextureOffsetClampARB";
        case EOpSparseTextureGradClamp: return "sparseTextureGradClampARB";
        case EOpSparseTextureGradOffsetClamp: return "sparseTextureGradOffsetClampARB";
        case EOpSparseTextureGatherLod: return "sparseTextureGatherLodAMD";
        case EOpSparseTextureGatherLodOffset: return "sparseTextureGatherLodOffsetAMD";
        case EOpSparseTextureGatherLodOffsets: return "sparseTextureGatherLodOffsetsAMD";
        case EOpImageSampleFootprintNV: return "textureFootprintNV";
        case EOpImageSampleFootprintClampNV: return "textureFootprintClampNV";
        case EOpImageSampleFootprintLodNV: return "textureFootprintLodNV";
        case EOpImageSampleFootprintGradNV: return "textureFootprintGradNV";
        case EOpImageSampleFootprintGradClampNV: return "textureFootprintGradClampNV";
        case EOpAddCarry: return "uaddCarry";
        case EOpSubBorrow: return "usubBorrow";
        case EOpUMulExtended: return "umulExtended";
        case EOpIMulExtended: return "imulExtended";
        case EOpBitfieldExtract: return "bitfieldExtract";
        case EOpBitfieldInsert: return "bitfieldInsert";
        case EOpBitFieldReverse: return "bitfieldReverse";
        case EOpBitCount: return "bitCount";
        case EOpFindLSB: return "findLSB";
        case EOpFindMSB: return "findMSB";
        case EOpCountLeadingZeros: return "countLeadingZeros";
        case EOpCountTrailingZeros: return "countTrailingZeros";
        case EOpAbsDifference: return "absoluteDifference";
        case EOpAddSaturate: return "addSaturate";
        case EOpSubSaturate: return "subtractSaturate";
        case EOpAverage: return "average";
        case EOpAverageRounded: return "averageRounded";
        case EOpMul32x16: return "multiply32x16";
        case EOpTraceNV: return "traceNV";
        case EOpTraceRayMotionNV: return "traceRayMotionNV";
        case EOpTraceKHR: return "traceRayEXT";
        case EOpReportIntersection: return "reportIntersectionEXT";
        case EOpIgnoreIntersectionNV: return "ignoreIntersectionNV";
        case EOpTerminateRayNV: return "terminateRayNV";
        case EOpExecuteCallableNV: return "executeCallableNV";
        case EOpExecuteCallableKHR: return "executeCallableEXT";
        case EOpWritePackedPrimitiveIndices4x8NV: return "writePackedPrimitiveIndices4x8NV";
        case EOpEmitMeshTasksEXT: return "EmitMeshTasksEXT";
        case EOpSetMeshOutputsEXT: return "SetMeshOutputsEXT";
        case EOpRayQueryInitialize: return "rayQueryInitializeEXT";
        case EOpRayQueryTerminate: return "rayQueryTerminateEXT";
        case EOpRayQueryGenerateIntersection: return "rayQueryGenerateIntersectionEXT";
        case EOpRayQueryConfirmIntersection: return "rayQueryConfirmIntersectionEXT";
        case EOpRayQueryProceed: return "rayQueryProceedEXT";
        case EOpRayQueryGetIntersectionType: return "rayQueryGetIntersectionTypeEXT";
        case EOpRayQueryGetRayTMin: return "rayQueryGetRayTMinEXT";
        case EOpRayQueryGetRayFlags: return "rayQueryGetRayFlagsEXT";
        case EOpRayQueryGetIntersectionT: return "rayQueryGetIntersectionTEXT";
        case EOpRayQueryGetIntersectionInstanceCustomIndex: return "rayQueryGetIntersectionInstanceCustomIndexEXT";
        case EOpRayQueryGetIntersectionInstanceId: return "rayQueryGetIntersectionInstanceIdEXT";
        case EOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffset: return "rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT";
        case EOpRayQueryGetIntersectionGeometryIndex: return "rayQueryGetIntersectionGeometryIndexEXT";
        case EOpRayQueryGetIntersectionPrimitiveIndex: return "rayQueryGetIntersectionPrimitiveIndexEXT";
        case EOpRayQueryGetIntersectionBarycentrics: return "rayQueryGetIntersectionBarycentricsEXT";
        case EOpRayQueryGetIntersectionFrontFace: return "rayQueryGetIntersectionFrontFaceEXT";
        case EOpRayQueryGetIntersectionCandidateAABBOpaque: return "rayQueryGetIntersectionCandidateAABBOpaqueEXT";
        case EOpRayQueryGetIntersectionObjectRayDirection: return "rayQueryGetIntersectionObjectRayDirectionEXT";
        case EOpRayQueryGetIntersectionObjectRayOrigin: return "rayQueryGetIntersectionObjectRayOriginEXT";
        case EOpRayQueryGetWorldRayDirection: return "rayQueryGetWorldRayDirectionEXT";
        case EOpRayQueryGetWorldRayOrigin: return "rayQueryGetWorldRayOriginEXT";
        case EOpRayQueryGetIntersectionObjectToWorld: return "rayQueryGetIntersectionObjectToWorldEXT";
        case EOpRayQueryGetIntersectionWorldToObject: return "rayQueryGetIntersectionWorldToObjectEXT";
        case EOpHitObjectTraceRayNV: return "hitObjectTraceRayNV";
        case EOpHitObjectTraceRayMotionNV: return "hitObjectTraceRayMotionNV";
        case EOpHitObjectRecordHitNV: return "hitObjectRecordHitNV";
        case EOpHitObjectRecordHitMotionNV: return "hitObjectRecordHitMotionNV";
        case EOpHitObjectRecordHitWithIndexNV: return "hitObjectRecordHitWithIndexNV";
        case EOpHitObjectRecordHitWithIndexMotionNV: return "hitObjectRecordHitWithIndexMotionNV";
        case EOpHitObjectRecordMissNV: return "hitObjectRecordMissNV";
        case EOpHitObjectRecordMissMotionNV: return "hitObjectRecordMissMotionNV";
        case EOpHitObjectRecordEmptyNV: return "hitObjectRecordEmptyNV";
        case EOpHitObjectExecuteShaderNV: return "hitObjectExecuteShaderNV";
        case EOpHitObjectIsEmptyNV: return "hitObjectIsEmptyNV";
        case EOpHitObjectIsMissNV: return "hitObjectIsMissNV";
        case EOpHitObjectIsHitNV: return "hitObjectIsHitNV";
        case EOpHitObjectGetRayTMinNV: return "hitObjectGetRayTMinNV";
        case EOpHitObjectGetRayTMaxNV: return "hitObjectGetRayTMaxNV";
        case EOpHitObjectGetObjectRayOriginNV: return "hitObjectGetObjectRayOriginNV";
        case EOpHitObjectGetObjectRayDirectionNV: return "hitObjectGetObjectRayDirectionNV";
        case EOpHitObjectGetWorldRayOriginNV: return "hitObjectGetWorldRayOriginNV";
        case EOpHitObjectGetWorldRayDirectionNV: return "hitObjectGetWorldRayDirectionNV";
        case EOpHitObjectGetWorldToObjectNV: return "hitObjectGetWorldToObjectNV";
        case EOpHitObjectGetObjectToWorldNV: return "hitObjectGetObjectToWorldNV";
        case EOpHitObjectGetInstanceCustomIndexNV: return "hitObjectGetInstanceCustomIndexNV";
        case EOpHitObjectGetInstanceIdNV: return "hitObjectGetInstanceIdNV";
        case EOpHitObjectGetGeometryIndexNV: return "hitObjectGetGeometryIndexNV";
        case EOpHitObjectGetPrimitiveIndexNV: return "hitObjectGetPrimitiveIndexNV";
        case EOpHitObjectGetHitKindNV: return "hitObjectGetHitKindNV";
        case EOpHitObjectGetShaderBindingTableRecordIndexNV: return "hitObjectGetShaderBindingTableRecordIndexNV";
        case EOpHitObjectGetShaderRecordBufferHandleNV: return "hitObjectGetShaderRecordBufferHandleNV";
        case EOpHitObjectGetAttributesNV: return "hitObjectGetAttributesNV";
        case EOpHitObjectGetCurrentTimeNV: return "hitObjectGetCurrentTimeNV";
        case EOpReorderThreadNV: return "reorderThreadNV";
        case EOpFetchMicroTriangleVertexPositionNV: return "fetchMicroTriangleVertexPositionNV";
        case EOpFetchMicroTriangleVertexBarycentricNV: return "fetchMicroTriangleVertexBarycentricNV";
        // case EOpReadClockSubgroupKHR: return "clock2x32ARB";
        // case EOpReadClockSubgroupKHR: return "clockARB";
        // case EOpReadClockDeviceKHR: return "clockRealtime2x32EXT";
        // case EOpReadClockDeviceKHR: return "clockRealtimeEXT";
        case EOpRayQueryGetIntersectionTriangleVertexPositionsEXT: return "rayQueryGetIntersectionTriangleVertexPositionsEXT";
        case EOpStencilAttachmentReadEXT: return "stencilAttachmentReadEXT";
        case EOpDepthAttachmentReadEXT: return "depthAttachmentReadEXT";
        case EOpImageSampleWeightedQCOM: return "textureWeightedQCOM";
        case EOpImageBoxFilterQCOM: return "textureBoxFilterQCOM";
        case EOpImageBlockMatchSADQCOM: return "textureBlockMatchSADQCOM";
        case EOpImageBlockMatchSSDQCOM: return "textureBlockMatchSSDQCOM";
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
    Slurper(const glslang::TIntermediate& intermediate) {
        mVersion = intermediate.getVersion();
        slurpFromRoot(intermediate.getTreeRoot()->getAsAggregate());
    }

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
    int mVersion;
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

    std::variant<RValueOperator, FunctionId> slurpOperator(
            glslang::TOperator op, Type returnType, std::optional<Type> arg1Type) {
        auto opOrFunctionName = glslangOperatorToRValueOperator(op, mVersion, returnType, arg1Type);
        if (auto* rValueOperator = std::get_if<RValueOperator>(&opOrFunctionName)) {
            return *rValueOperator;
        }
        if (auto* functionName = std::get_if<std::string_view>(&opOrFunctionName)) {
            return mFunctionNames.getOrInsertByValue(*functionName);
        }
        PANIC_POSTCONDITION("Unreachable");
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
            auto operandId = slurpValue(nodeAsUnary->getOperand(), node);
            auto op = slurpOperator(
                    nodeAsUnary->getOp(),
                    glslangTypeToType(node->getType()),
                    glslangTypeToType(nodeAsUnary->getOperand()->getType()));
            return mRValues.getOrInsertByValue(EvaluableRValue{op, {operandId}});
        }
        if (auto nodeAsBinary = node->getAsBinaryNode()) {
            switch (nodeAsBinary->getOp()) {
                case glslang::EOpVectorSwizzle: {
                    // TODO: swizzle it up
                    auto swizzle = nodeAsBinary->getRight()->getAsAggregate();
                    ASSERT_PRECONDITION(swizzle != nullptr, "Swizzle node must be an aggregate");
                    ASSERT_PRECONDITION(swizzle->getOp() == glslang::EOpSequence, "Swizzle node must be a sequence");
                    return mRValues.getOrInsertByValue(EvaluableRValue{RValueOperator::VectorSwizzle});
                }
                default: {
                    auto lhsId = slurpValue(nodeAsBinary->getLeft(), node);
                    auto rhsId = slurpValue(nodeAsBinary->getRight(), node);
                    auto op = slurpOperator(
                            nodeAsBinary->getOp(),
                            glslangTypeToType(node->getType()),
                            glslangTypeToType(nodeAsBinary->getLeft()->getType()));
                    return mRValues.getOrInsertByValue(EvaluableRValue{op, {lhsId, rhsId}});
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
                return mRValues.getOrInsertByValue(EvaluableRValue{RValueOperator::Ternary, {conditionId, trueId, falseId}});
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
                            EvaluableRValue{functionId, std::move(args)});
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
                    std::optional<Type> firstArgType;
                    if (!sequence.empty()) {
                        firstArgType = glslangTypeToType(sequence[0]->getAsTyped()->getType());
                    }
                    auto op = slurpOperator(
                            nodeAsAggregate->getOp(),
                            glslangTypeToType(node->getType()),
                            std::move(firstArgType));
                    return mRValues.getOrInsertByValue(
                            EvaluableRValue{op, std::move(args)});
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
        if (auto* evaluable = std::get_if<EvaluableRValue>(&rValue)) {
            if (auto* op = std::get_if<RValueOperator>(&evaluable->op)) {
                out << "(" << rValueOperatorToString(*op);
            } else if (auto* function = std::get_if<FunctionId>(&evaluable->op)) {
                auto name = mFunctionNames.getById(*function);
                out << name << "(";
            } else {
                PANIC_PRECONDITION("Unreachable");
            }
            for (auto& arg : evaluable->args) {
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
    Slurper slurper(intermediate);
    std::stringstream glsl;
    slurper.dumpAll(glsl);
    utils::slog.i << std::move(glsl.str()) << utils::io::endl;
}

} // namespace glslkomi
