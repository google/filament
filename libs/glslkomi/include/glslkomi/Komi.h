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

#ifndef TNT_GLSLKOMI_KOMI_H
#define TNT_GLSLKOMI_KOMI_H

#include <utils/compiler.h>
#include <vector>
#include <variant>
#include "intermediate.h"
#include <backend/DriverEnums.h>
#include <ShaderLang.h>

namespace glslkomi {

enum class BranchOperator : uint8_t {
    Discard,             // Fragment only
    TerminateInvocation, // Fragment only
    Demote,              // Fragment only
    TerminateRayEXT,         // Any-hit only
    IgnoreIntersectionEXT,   // Any-hit only
    Return,
    Break,
    Continue,
    Case,
    Default,
};

enum class RValueOperator : uint16_t {
    Ternary,

    //
    // Unary operators
    //

    Negative,
    LogicalNot,
    VectorLogicalNot,
    BitwiseNot,

    PostIncrement,
    PostDecrement,
    PreIncrement,
    PreDecrement,

    CopyObject,

    Declare,        // Used by debugging to force declaration of variable in correct scope

    // (u)int* -> bool
    ConvInt8ToBool,
    ConvUint8ToBool,
    ConvInt16ToBool,
    ConvUint16ToBool,
    ConvIntToBool,
    ConvUintToBool,
    ConvInt64ToBool,
    ConvUint64ToBool,

    // float* -> bool
    ConvFloat16ToBool,
    ConvFloatToBool,
    ConvDoubleToBool,

    // bool -> (u)int*
    ConvBoolToInt8,
    ConvBoolToUint8,
    ConvBoolToInt16,
    ConvBoolToUint16,
    ConvBoolToInt,
    ConvBoolToUint,
    ConvBoolToInt64,
    ConvBoolToUint64,

    // bool -> float*
    ConvBoolToFloat16,
    ConvBoolToFloat,
    ConvBoolToDouble,

    // int8_t -> (u)int*
    ConvInt8ToInt16,
    ConvInt8ToInt,
    ConvInt8ToInt64,
    ConvInt8ToUint8,
    ConvInt8ToUint16,
    ConvInt8ToUint,
    ConvInt8ToUint64,

    // uint8_t -> (u)int*
    ConvUint8ToInt8,
    ConvUint8ToInt16,
    ConvUint8ToInt,
    ConvUint8ToInt64,
    ConvUint8ToUint16,
    ConvUint8ToUint,
    ConvUint8ToUint64,

    // int8_t -> float*
    ConvInt8ToFloat16,
    ConvInt8ToFloat,
    ConvInt8ToDouble,

    // uint8_t -> float*
    ConvUint8ToFloat16,
    ConvUint8ToFloat,
    ConvUint8ToDouble,

    // int16_t -> (u)int*
    ConvInt16ToInt8,
    ConvInt16ToInt,
    ConvInt16ToInt64,
    ConvInt16ToUint8,
    ConvInt16ToUint16,
    ConvInt16ToUint,
    ConvInt16ToUint64,

    // uint16_t -> (u)int*
    ConvUint16ToInt8,
    ConvUint16ToInt16,
    ConvUint16ToInt,
    ConvUint16ToInt64,
    ConvUint16ToUint8,
    ConvUint16ToUint,
    ConvUint16ToUint64,

    // int16_t -> float*
    ConvInt16ToFloat16,
    ConvInt16ToFloat,
    ConvInt16ToDouble,

    // uint16_t -> float*
    ConvUint16ToFloat16,
    ConvUint16ToFloat,
    ConvUint16ToDouble,

    // int32_t -> (u)int*
    ConvIntToInt8,
    ConvIntToInt16,
    ConvIntToInt64,
    ConvIntToUint8,
    ConvIntToUint16,
    ConvIntToUint,
    ConvIntToUint64,

    // uint32_t -> (u)int*
    ConvUintToInt8,
    ConvUintToInt16,
    ConvUintToInt,
    ConvUintToInt64,
    ConvUintToUint8,
    ConvUintToUint16,
    ConvUintToUint64,

    // int32_t -> float*
    ConvIntToFloat16,
    ConvIntToFloat,
    ConvIntToDouble,

    // uint32_t -> float*
    ConvUintToFloat16,
    ConvUintToFloat,
    ConvUintToDouble,

    // int64_t -> (u)int*
    ConvInt64ToInt8,
    ConvInt64ToInt16,
    ConvInt64ToInt,
    ConvInt64ToUint8,
    ConvInt64ToUint16,
    ConvInt64ToUint,
    ConvInt64ToUint64,

    // uint64_t -> (u)int*
    ConvUint64ToInt8,
    ConvUint64ToInt16,
    ConvUint64ToInt,
    ConvUint64ToInt64,
    ConvUint64ToUint8,
    ConvUint64ToUint16,
    ConvUint64ToUint,

    // int64_t -> float*
    ConvInt64ToFloat16,
    ConvInt64ToFloat,
    ConvInt64ToDouble,

    // uint64_t -> float*
    ConvUint64ToFloat16,
    ConvUint64ToFloat,
    ConvUint64ToDouble,

    // float16_t -> (u)int*
    ConvFloat16ToInt8,
    ConvFloat16ToInt16,
    ConvFloat16ToInt,
    ConvFloat16ToInt64,
    ConvFloat16ToUint8,
    ConvFloat16ToUint16,
    ConvFloat16ToUint,
    ConvFloat16ToUint64,

    // float16_t -> float*
    ConvFloat16ToFloat,
    ConvFloat16ToDouble,

    // float -> (u)int*
    ConvFloatToInt8,
    ConvFloatToInt16,
    ConvFloatToInt,
    ConvFloatToInt64,
    ConvFloatToUint8,
    ConvFloatToUint16,
    ConvFloatToUint,
    ConvFloatToUint64,

    // float -> float*
    ConvFloatToFloat16,
    ConvFloatToDouble,

    // float64 _t-> (u)int*
    ConvDoubleToInt8,
    ConvDoubleToInt16,
    ConvDoubleToInt,
    ConvDoubleToInt64,
    ConvDoubleToUint8,
    ConvDoubleToUint16,
    ConvDoubleToUint,
    ConvDoubleToUint64,

    // float64_t -> float*
    ConvDoubleToFloat16,
    ConvDoubleToFloat,

    // uint64_t <-> pointer
    ConvUint64ToPtr,
    ConvPtrToUint64,

    // uvec2 <-> pointer
    ConvUvec2ToPtr,
    ConvPtrToUvec2,

    // uint64_t -> accelerationStructureEXT
    ConvUint64ToAccStruct,

    // uvec2 -> accelerationStructureEXT
    ConvUvec2ToAccStruct,

    //
    // binary operations
    //

    Add,
    Sub,
    Mul,
    Div,
    Mod,
    RightShift,
    LeftShift,
    And,
    InclusiveOr,
    ExclusiveOr,
    Equal,
    NotEqual,
    VectorEqual,
    VectorNotEqual,
    LessThan,
    GreaterThan,
    LessThanEqual,
    GreaterThanEqual,
    Comma,

    VectorTimesScalar,
    VectorTimesMatrix,
    MatrixTimesVector,
    MatrixTimesScalar,

    LogicalOr,
    LogicalXor,
    LogicalAnd,

    IndexDirect,
    IndexIndirect,
    IndexDirectStruct,

    VectorSwizzle,

    Method,
    Scoping,

    //
    // Built-in functions mapped to operators
    //

    Radians,
    Degrees,
    Sin,
    Cos,
    Tan,
    Asin,
    Acos,
    Atan,
    Sinh,
    Cosh,
    Tanh,
    Asinh,
    Acosh,
    Atanh,

    Pow,
    Exp,
    Log,
    Exp2,
    Log2,
    Sqrt,
    InverseSqrt,

    Abs,
    Sign,
    Floor,
    Trunc,
    Round,
    RoundEven,
    Ceil,
    Fract,
    Modf,
    Min,
    Max,
    Clamp,
    Mix,
    Step,
    SmoothStep,

    IsNan,
    IsInf,

    Fma,

    Frexp,
    Ldexp,

    FloatBitsToInt,
    FloatBitsToUint,
    IntBitsToFloat,
    UintBitsToFloat,
    DoubleBitsToInt64,
    DoubleBitsToUint64,
    Int64BitsToDouble,
    Uint64BitsToDouble,
    Float16BitsToInt16,
    Float16BitsToUint16,
    Int16BitsToFloat16,
    Uint16BitsToFloat16,
    PackSnorm2x16,
    UnpackSnorm2x16,
    PackUnorm2x16,
    UnpackUnorm2x16,
    PackSnorm4x8,
    UnpackSnorm4x8,
    PackUnorm4x8,
    UnpackUnorm4x8,
    PackHalf2x16,
    UnpackHalf2x16,
    PackDouble2x32,
    UnpackDouble2x32,
    PackInt2x32,
    UnpackInt2x32,
    PackUint2x32,
    UnpackUint2x32,
    PackFloat2x16,
    UnpackFloat2x16,
    PackInt2x16,
    UnpackInt2x16,
    PackUint2x16,
    UnpackUint2x16,
    PackInt4x16,
    UnpackInt4x16,
    PackUint4x16,
    UnpackUint4x16,
    Pack16,
    Pack32,
    Pack64,
    Unpack32,
    Unpack16,
    Unpack8,

    Length,
    Distance,
    Dot,
    Cross,
    Normalize,
    FaceForward,
    Reflect,
    Refract,

    Min3,
    Max3,
    Mid3,

    DPdx,            // Fragment only
    DPdy,            // Fragment only
    Fwidth,          // Fragment only
    DPdxFine,        // Fragment only
    DPdyFine,        // Fragment only
    FwidthFine,      // Fragment only
    DPdxCoarse,      // Fragment only
    DPdyCoarse,      // Fragment only
    FwidthCoarse,    // Fragment only

    InterpolateAtCentroid, // Fragment only
    InterpolateAtSample,   // Fragment only
    InterpolateAtOffset,   // Fragment only
    InterpolateAtVertex,

    MatrixTimesMatrix,
    OuterProduct,
    Determinant,
    MatrixInverse,
    Transpose,

    Ftransform,

    Noise,

    EmitVertex,           // geometry only
    EndPrimitive,         // geometry only
    EmitStreamVertex,     // geometry only
    EndStreamPrimitive,   // geometry only

    Barrier,
    MemoryBarrier,
    MemoryBarrierAtomicCounter,
    MemoryBarrierBuffer,
    MemoryBarrierImage,
    MemoryBarrierShared,  // compute only
    GroupMemoryBarrier,   // compute only

    Ballot,
    ReadInvocation,
    ReadFirstInvocation,

    AnyInvocation,
    AllInvocations,
    AllInvocationsEqual,

    SubgroupGuardStart,
    SubgroupBarrier,
    SubgroupMemoryBarrier,
    SubgroupMemoryBarrierBuffer,
    SubgroupMemoryBarrierImage,
    SubgroupMemoryBarrierShared, // compute only
    SubgroupElect,
    SubgroupAll,
    SubgroupAny,
    SubgroupAllEqual,
    SubgroupBroadcast,
    SubgroupBroadcastFirst,
    SubgroupBallot,
    SubgroupInverseBallot,
    SubgroupBallotBitExtract,
    SubgroupBallotBitCount,
    SubgroupBallotInclusiveBitCount,
    SubgroupBallotExclusiveBitCount,
    SubgroupBallotFindLSB,
    SubgroupBallotFindMSB,
    SubgroupShuffle,
    SubgroupShuffleXor,
    SubgroupShuffleUp,
    SubgroupShuffleDown,
    SubgroupAdd,
    SubgroupMul,
    SubgroupMin,
    SubgroupMax,
    SubgroupAnd,
    SubgroupOr,
    SubgroupXor,
    SubgroupInclusiveAdd,
    SubgroupInclusiveMul,
    SubgroupInclusiveMin,
    SubgroupInclusiveMax,
    SubgroupInclusiveAnd,
    SubgroupInclusiveOr,
    SubgroupInclusiveXor,
    SubgroupExclusiveAdd,
    SubgroupExclusiveMul,
    SubgroupExclusiveMin,
    SubgroupExclusiveMax,
    SubgroupExclusiveAnd,
    SubgroupExclusiveOr,
    SubgroupExclusiveXor,
    SubgroupClusteredAdd,
    SubgroupClusteredMul,
    SubgroupClusteredMin,
    SubgroupClusteredMax,
    SubgroupClusteredAnd,
    SubgroupClusteredOr,
    SubgroupClusteredXor,
    SubgroupQuadBroadcast,
    SubgroupQuadSwapHorizontal,
    SubgroupQuadSwapVertical,
    SubgroupQuadSwapDiagonal,

    SubgroupPartition,
    SubgroupPartitionedAdd,
    SubgroupPartitionedMul,
    SubgroupPartitionedMin,
    SubgroupPartitionedMax,
    SubgroupPartitionedAnd,
    SubgroupPartitionedOr,
    SubgroupPartitionedXor,
    SubgroupPartitionedInclusiveAdd,
    SubgroupPartitionedInclusiveMul,
    SubgroupPartitionedInclusiveMin,
    SubgroupPartitionedInclusiveMax,
    SubgroupPartitionedInclusiveAnd,
    SubgroupPartitionedInclusiveOr,
    SubgroupPartitionedInclusiveXor,
    SubgroupPartitionedExclusiveAdd,
    SubgroupPartitionedExclusiveMul,
    SubgroupPartitionedExclusiveMin,
    SubgroupPartitionedExclusiveMax,
    SubgroupPartitionedExclusiveAnd,
    SubgroupPartitionedExclusiveOr,
    SubgroupPartitionedExclusiveXor,

    SubgroupGuardStop,

    MinInvocations,
    MaxInvocations,
    AddInvocations,
    MinInvocationsNonUniform,
    MaxInvocationsNonUniform,
    AddInvocationsNonUniform,
    MinInvocationsInclusiveScan,
    MaxInvocationsInclusiveScan,
    AddInvocationsInclusiveScan,
    MinInvocationsInclusiveScanNonUniform,
    MaxInvocationsInclusiveScanNonUniform,
    AddInvocationsInclusiveScanNonUniform,
    MinInvocationsExclusiveScan,
    MaxInvocationsExclusiveScan,
    AddInvocationsExclusiveScan,
    MinInvocationsExclusiveScanNonUniform,
    MaxInvocationsExclusiveScanNonUniform,
    AddInvocationsExclusiveScanNonUniform,
    SwizzleInvocations,
    SwizzleInvocationsMasked,
    WriteInvocation,
    Mbcnt,

    CubeFaceIndex,
    CubeFaceCoord,
    Time,

    AtomicAdd,
    AtomicSubtract,
    AtomicMin,
    AtomicMax,
    AtomicAnd,
    AtomicOr,
    AtomicXor,
    AtomicExchange,
    AtomicCompSwap,
    AtomicLoad,
    AtomicStore,

    AtomicCounterIncrement, // results in pre-increment value
    AtomicCounterDecrement, // results in post-decrement value
    AtomicCounter,
    AtomicCounterAdd,
    AtomicCounterSubtract,
    AtomicCounterMin,
    AtomicCounterMax,
    AtomicCounterAnd,
    AtomicCounterOr,
    AtomicCounterXor,
    AtomicCounterExchange,
    AtomicCounterCompSwap,

    Any,
    All,

    CooperativeMatrixLoad,
    CooperativeMatrixStore,
    CooperativeMatrixMulAdd,
    CooperativeMatrixLoadNV,
    CooperativeMatrixStoreNV,
    CooperativeMatrixMulAddNV,

    BeginInvocationInterlock, // Fragment only
    EndInvocationInterlock, // Fragment only

    IsHelperInvocation,

    DebugPrintf,

    //
    // Constructors
    //

    ConstructGuardStart,
    ConstructInt,          // these first scalar forms also identify what implicit conversion is needed
    ConstructUint,
    ConstructInt8,
    ConstructUint8,
    ConstructInt16,
    ConstructUint16,
    ConstructInt64,
    ConstructUint64,
    ConstructBool,
    ConstructFloat,
    ConstructDouble,
    // Keep vector and matrix constructors in a consistent relative order for
    // TParseContext::constructBuiltIn, which converts between 8/16/32 bit
    // vector constructors
    ConstructVec2,
    ConstructVec3,
    ConstructVec4,
    ConstructMat2x2,
    ConstructMat2x3,
    ConstructMat2x4,
    ConstructMat3x2,
    ConstructMat3x3,
    ConstructMat3x4,
    ConstructMat4x2,
    ConstructMat4x3,
    ConstructMat4x4,
    ConstructDVec2,
    ConstructDVec3,
    ConstructDVec4,
    ConstructBVec2,
    ConstructBVec3,
    ConstructBVec4,
    ConstructI8Vec2,
    ConstructI8Vec3,
    ConstructI8Vec4,
    ConstructU8Vec2,
    ConstructU8Vec3,
    ConstructU8Vec4,
    ConstructI16Vec2,
    ConstructI16Vec3,
    ConstructI16Vec4,
    ConstructU16Vec2,
    ConstructU16Vec3,
    ConstructU16Vec4,
    ConstructIVec2,
    ConstructIVec3,
    ConstructIVec4,
    ConstructUVec2,
    ConstructUVec3,
    ConstructUVec4,
    ConstructI64Vec2,
    ConstructI64Vec3,
    ConstructI64Vec4,
    ConstructU64Vec2,
    ConstructU64Vec3,
    ConstructU64Vec4,
    ConstructDMat2x2,
    ConstructDMat2x3,
    ConstructDMat2x4,
    ConstructDMat3x2,
    ConstructDMat3x3,
    ConstructDMat3x4,
    ConstructDMat4x2,
    ConstructDMat4x3,
    ConstructDMat4x4,
    ConstructIMat2x2,
    ConstructIMat2x3,
    ConstructIMat2x4,
    ConstructIMat3x2,
    ConstructIMat3x3,
    ConstructIMat3x4,
    ConstructIMat4x2,
    ConstructIMat4x3,
    ConstructIMat4x4,
    ConstructUMat2x2,
    ConstructUMat2x3,
    ConstructUMat2x4,
    ConstructUMat3x2,
    ConstructUMat3x3,
    ConstructUMat3x4,
    ConstructUMat4x2,
    ConstructUMat4x3,
    ConstructUMat4x4,
    ConstructBMat2x2,
    ConstructBMat2x3,
    ConstructBMat2x4,
    ConstructBMat3x2,
    ConstructBMat3x3,
    ConstructBMat3x4,
    ConstructBMat4x2,
    ConstructBMat4x3,
    ConstructBMat4x4,
    ConstructFloat16,
    ConstructF16Vec2,
    ConstructF16Vec3,
    ConstructF16Vec4,
    ConstructF16Mat2x2,
    ConstructF16Mat2x3,
    ConstructF16Mat2x4,
    ConstructF16Mat3x2,
    ConstructF16Mat3x3,
    ConstructF16Mat3x4,
    ConstructF16Mat4x2,
    ConstructF16Mat4x3,
    ConstructF16Mat4x4,
    ConstructStruct,
    ConstructTextureSampler,
    ConstructNonuniform,     // expected to be transformed away, not present in final AST
    ConstructReference,
    ConstructCooperativeMatrixNV,
    ConstructCooperativeMatrixKHR,
    ConstructAccStruct,
    ConstructGuardEnd,

    //
    // moves
    //

    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    VectorTimesMatrixAssign,
    VectorTimesScalarAssign,
    MatrixTimesScalarAssign,
    MatrixTimesMatrixAssign,
    DivAssign,
    ModAssign,
    AndAssign,
    InclusiveOrAssign,
    ExclusiveOrAssign,
    LeftShiftAssign,
    RightShiftAssign,

    //
    // Array operators
    //

    // Can apply to arrays, vectors, or matrices.
    // Can be decomposed to a constant at compile time, but this does not always happen,
    // due to link-time effects. So, consumer can expect either a link-time sized or
    // run-time sized array.
    ArrayLength,

    //
    // Image operations
    //

    ImageGuardBegin,

    ImageQuerySize,
    ImageQuerySamples,
    ImageLoad,
    ImageStore,
    ImageLoadLod,
    ImageStoreLod,
    ImageAtomicAdd,
    ImageAtomicMin,
    ImageAtomicMax,
    ImageAtomicAnd,
    ImageAtomicOr,
    ImageAtomicXor,
    ImageAtomicExchange,
    ImageAtomicCompSwap,
    ImageAtomicLoad,
    ImageAtomicStore,

    SubpassLoad,
    SubpassLoadMS,
    SparseImageLoad,
    SparseImageLoadLod,
    ColorAttachmentReadEXT, // Fragment only

    ImageGuardEnd,

    //
    // Texture operations
    //

    TextureGuardBegin,

    TextureQuerySize,
    TextureQueryLod,
    TextureQueryLevels,
    TextureQuerySamples,

    SamplingGuardBegin,

    Texture,
    TextureProj,
    TextureLod,
    TextureOffset,
    TextureFetch,
    TextureFetchOffset,
    TextureProjOffset,
    TextureLodOffset,
    TextureProjLod,
    TextureProjLodOffset,
    TextureGrad,
    TextureGradOffset,
    TextureProjGrad,
    TextureProjGradOffset,
    TextureGather,
    TextureGatherOffset,
    TextureGatherOffsets,
    TextureClamp,
    TextureOffsetClamp,
    TextureGradClamp,
    TextureGradOffsetClamp,
    TextureGatherLod,
    TextureGatherLodOffset,
    TextureGatherLodOffsets,
    FragmentMaskFetch,
    FragmentFetch,

    SparseTextureGuardBegin,

    SparseTexture,
    SparseTextureLod,
    SparseTextureOffset,
    SparseTextureFetch,
    SparseTextureFetchOffset,
    SparseTextureLodOffset,
    SparseTextureGrad,
    SparseTextureGradOffset,
    SparseTextureGather,
    SparseTextureGatherOffset,
    SparseTextureGatherOffsets,
    SparseTexelsResident,
    SparseTextureClamp,
    SparseTextureOffsetClamp,
    SparseTextureGradClamp,
    SparseTextureGradOffsetClamp,
    SparseTextureGatherLod,
    SparseTextureGatherLodOffset,
    SparseTextureGatherLodOffsets,

    SparseTextureGuardEnd,

    ImageFootprintGuardBegin,
    ImageSampleFootprintNV,
    ImageSampleFootprintClampNV,
    ImageSampleFootprintLodNV,
    ImageSampleFootprintGradNV,
    ImageSampleFootprintGradClampNV,
    ImageFootprintGuardEnd,
    SamplingGuardEnd,
    TextureGuardEnd,

    //
    // Integer operations
    //

    AddCarry,
    SubBorrow,
    UMulExtended,
    IMulExtended,
    BitfieldExtract,
    BitfieldInsert,
    BitFieldReverse,
    BitCount,
    FindLSB,
    FindMSB,

    CountLeadingZeros,
    CountTrailingZeros,
    AbsDifference,
    AddSaturate,
    SubSaturate,
    Average,
    AverageRounded,
    Mul32x16,

    TraceNV,
    TraceRayMotionNV,
    TraceKHR,
    ReportIntersection,
    IgnoreIntersectionNV,
    TerminateRayNV,
    ExecuteCallableNV,
    ExecuteCallableKHR,
    WritePackedPrimitiveIndices4x8NV,
    EmitMeshTasksEXT,
    SetMeshOutputsEXT,

    //
    // GL_EXT_ray_query operations
    //

    RayQueryInitialize,
    RayQueryTerminate,
    RayQueryGenerateIntersection,
    RayQueryConfirmIntersection,
    RayQueryProceed,
    RayQueryGetIntersectionType,
    RayQueryGetRayTMin,
    RayQueryGetRayFlags,
    RayQueryGetIntersectionT,
    RayQueryGetIntersectionInstanceCustomIndex,
    RayQueryGetIntersectionInstanceId,
    RayQueryGetIntersectionInstanceShaderBindingTableRecordOffset,
    RayQueryGetIntersectionGeometryIndex,
    RayQueryGetIntersectionPrimitiveIndex,
    RayQueryGetIntersectionBarycentrics,
    RayQueryGetIntersectionFrontFace,
    RayQueryGetIntersectionCandidateAABBOpaque,
    RayQueryGetIntersectionObjectRayDirection,
    RayQueryGetIntersectionObjectRayOrigin,
    RayQueryGetWorldRayDirection,
    RayQueryGetWorldRayOrigin,
    RayQueryGetIntersectionObjectToWorld,
    RayQueryGetIntersectionWorldToObject,

    //
    // GL_NV_shader_invocation_reorder
    //

    HitObjectTraceRayNV,
    HitObjectTraceRayMotionNV,
    HitObjectRecordHitNV,
    HitObjectRecordHitMotionNV,
    HitObjectRecordHitWithIndexNV,
    HitObjectRecordHitWithIndexMotionNV,
    HitObjectRecordMissNV,
    HitObjectRecordMissMotionNV,
    HitObjectRecordEmptyNV,
    HitObjectExecuteShaderNV,
    HitObjectIsEmptyNV,
    HitObjectIsMissNV,
    HitObjectIsHitNV,
    HitObjectGetRayTMinNV,
    HitObjectGetRayTMaxNV,
    HitObjectGetObjectRayOriginNV,
    HitObjectGetObjectRayDirectionNV,
    HitObjectGetWorldRayOriginNV,
    HitObjectGetWorldRayDirectionNV,
    HitObjectGetWorldToObjectNV,
    HitObjectGetObjectToWorldNV,
    HitObjectGetInstanceCustomIndexNV,
    HitObjectGetInstanceIdNV,
    HitObjectGetGeometryIndexNV,
    HitObjectGetPrimitiveIndexNV,
    HitObjectGetHitKindNV,
    HitObjectGetShaderBindingTableRecordIndexNV,
    HitObjectGetShaderRecordBufferHandleNV,
    HitObjectGetAttributesNV,
    HitObjectGetCurrentTimeNV,
    ReorderThreadNV,
    FetchMicroTriangleVertexPositionNV,
    FetchMicroTriangleVertexBarycentricNV,
};

// The order of the fields in this enum is very important due to a hack in glslangTypeToType.
enum class BuiltInType : uint8_t {
    Void,
    Struct,
    Block,
    Sampler2DArray,
    // Float
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat2,
    Mat2x3,
    Mat2x4,
    Mat3x2,
    Mat3,
    Mat3x4,
    Mat4x2,
    Mat4x3,
    Mat4,
    // Double
    Double,
    Dvec2,
    Dvec3,
    Dvec4,
    Dmat2,
    Dmat2x3,
    Dmat2x4,
    Dmat3x2,
    Dmat3,
    Dmat3x4,
    Dmat4x2,
    Dmat4x3,
    Dmat4,
    // Int
    Int,
    IVec2,
    IVec3,
    IVec4,
    // UInt
    Uint,
    Uvec2,
    Uvec3,
    Uvec4,
    // Bool
    Bool,
    Bvec2,
    Bvec3,
    Bvec4,
    // AtomicUInt
    AtomicUint,
};

// // Workaround for missing std::expected.
// template<typename T>
// using StatusOr = std::variant<T, int>;

using Precision = filament::backend::Precision;
using ElementType = filament::backend::ElementType;

template<typename T>
struct Id {
    std::size_t id;

    bool operator==(const Id<T>& o) const {
        return id == o.id;
    };
};

using TypeId = Id<struct TypeIdTag>;
using LValueId = Id<struct LValueTag>;
using RValueId = Id<struct RValueTag>;
using ValueId = std::variant<LValueId, RValueId>;
using FunctionId = Id<struct FunctionIdTag>;
using StatementBlockId = Id<struct StatementBlockIdTag>;

template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

// TODO: qualifiers
struct Type {
    std::string_view name;
    std::string_view precision;
    std::vector<std::size_t> arraySizes;

    bool operator==(const Type& o) const {
        return name == o.name
                && precision == o.precision
                && arraySizes == o.arraySizes;
    }
};

struct LValue {
    std::string_view name;

    bool operator==(const LValue& o) const {
        return name == o.name;
    }
};

struct FunctionCallRValue {
    FunctionId function;
    std::vector<ValueId> args;

    bool operator==(const FunctionCallRValue& o) const {
        return function == o.function
                && args == o.args;
    }
};

struct OperatorRValue {
    RValueOperator op;
    std::vector<ValueId> args;

    bool operator==(const OperatorRValue& o) const {
        return op == o.op
                && args == o.args;
    }
};

// TODO: value
struct LiteralRValue {
    bool operator==(const LiteralRValue& o) const {
        return true;
    }
};

using RValue = std::variant<
    FunctionCallRValue,
    OperatorRValue,
    LiteralRValue>;

// TODO: annotation
struct FunctionParameter {
    LValueId name;
    TypeId type;

    bool operator==(const FunctionParameter& o) const {
        return name == o.name
                && type == o.type;
    }
};

// TODO: type, local variables
struct FunctionDefinition {
    FunctionId name;
    TypeId returnType;
    std::vector<FunctionParameter> parameters;
    std::optional<StatementBlockId> body;

    bool operator==(const FunctionDefinition& o) const {
        return name == o.name
                && returnType == o.returnType
                && parameters == o.parameters
                && body == o.body;
    }
};

struct IfStatement {
    ValueId condition;
    StatementBlockId thenBlock;
    std::optional<StatementBlockId> elseBlock;

    bool operator==(const IfStatement& o) const {
        return condition == o.condition
                && thenBlock == o.thenBlock
                && elseBlock == o.elseBlock;
    }
};

struct SwitchStatement {
    ValueId condition;
    StatementBlockId body;

    bool operator==(const SwitchStatement& o) const {
        return condition == o.condition
                && body == o.body;
    }
};

struct BranchStatement {
    BranchOperator op;
    std::optional<ValueId> operand;

    bool operator==(const BranchStatement& o) const {
        return op == o.op
                && operand == o.operand;
    }
};

struct LoopStatement {
    ValueId condition;
    std::optional<RValueId> terminal;
    bool testFirst;
    StatementBlockId body;

    bool operator==(const LoopStatement& o) const {
        return condition == o.condition
                && terminal == o.terminal
                && testFirst == o.testFirst
                && body == o.body;
    }
};

using Statement = std::variant<
    RValueId,
    IfStatement,
    SwitchStatement,
    BranchStatement,
    LoopStatement>;

class Komi {
public:
    void slurp(const glslang::TIntermediate& intermediate);
};

} // namespace glslkomi

template<typename T>
struct ::std::hash<glslkomi::Id<T>> {
    std::size_t operator()(const glslkomi::Id<T>& o) const {
        return o.id;
    }
};


template<>
struct ::std::hash<glslkomi::Type> {
    std::size_t operator()(const glslkomi::Type& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.precision, o.arraySizes.size());
        for (const auto& size : o.arraySizes) {
            glslkomi::hashCombine(result, size);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::LValue> {
    std::size_t operator()(const glslkomi::LValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionCallRValue> {
    std::size_t operator()(const glslkomi::FunctionCallRValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.function, o.args.size());
        for (const auto& arg : o.args) {
            glslkomi::hashCombine(result, arg);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::OperatorRValue> {
    std::size_t operator()(const glslkomi::OperatorRValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.op, o.args.size());
        for (const auto& arg : o.args) {
            glslkomi::hashCombine(result, arg);
        }
        return result;
    }
};

// TODO
template<>
struct ::std::hash<glslkomi::LiteralRValue> {
    std::size_t operator()(const glslkomi::LiteralRValue& o) const {
        return 0;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionParameter> {
    std::size_t operator()(const glslkomi::FunctionParameter& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.type);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionDefinition> {
    std::size_t operator()(const glslkomi::FunctionDefinition& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.returnType, o.body, o.parameters.size());
        for (const auto& argument : o.parameters) {
            glslkomi::hashCombine(result, argument);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::IfStatement> {
    std::size_t operator()(const glslkomi::IfStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.thenBlock, o.elseBlock);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::SwitchStatement> {
    std::size_t operator()(const glslkomi::SwitchStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.body);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::BranchStatement> {
    std::size_t operator()(const glslkomi::BranchStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.op, o.operand);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::LoopStatement> {
    std::size_t operator()(const glslkomi::LoopStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.terminal, o.testFirst, o.body);
        return result;
    }
};

template<>
struct ::std::hash<std::vector<glslkomi::Statement>> {
    std::size_t operator()(const std::vector<glslkomi::Statement>& o) const {
        std::size_t result = o.size();
        for (const auto& statement : o) {
            glslkomi::hashCombine(result, statement);
        }
        return result;
    }
};

#endif
