// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_grammar_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2024 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is related to anything that is found in the SPIR-V grammar
 * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
 *
 ****************************************************************************/

// NOLINTBEGIN

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <spirv/unified1/spirv.hpp>

const char* string_SpvOpcode(uint32_t opcode);
const char* string_SpvStorageClass(uint32_t storage_class);
const char* string_SpvExecutionModel(uint32_t execution_model);
const char* string_SpvExecutionMode(uint32_t execution_mode);
const char* string_SpvDecoration(uint32_t decoration);
const char* string_SpvBuiltIn(uint32_t built_in);
const char* string_SpvDim(uint32_t dim);
std::string string_SpvCooperativeMatrixOperands(uint32_t mask);

static constexpr bool OpcodeHasType(uint32_t opcode) {
    switch (opcode) {
        case spv::OpUndef:
        case spv::OpExtInst:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstant:
        case spv::OpConstantComposite:
        case spv::OpConstantNull:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstant:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:
        case spv::OpFunction:
        case spv::OpFunctionParameter:
        case spv::OpFunctionCall:
        case spv::OpVariable:
        case spv::OpImageTexelPointer:
        case spv::OpLoad:
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
        case spv::OpPtrAccessChain:
        case spv::OpArrayLength:
        case spv::OpInBoundsPtrAccessChain:
        case spv::OpVectorExtractDynamic:
        case spv::OpVectorInsertDynamic:
        case spv::OpVectorShuffle:
        case spv::OpCompositeConstruct:
        case spv::OpCompositeExtract:
        case spv::OpCompositeInsert:
        case spv::OpCopyObject:
        case spv::OpTranspose:
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpConvertFToU:
        case spv::OpConvertFToS:
        case spv::OpConvertSToF:
        case spv::OpConvertUToF:
        case spv::OpUConvert:
        case spv::OpSConvert:
        case spv::OpFConvert:
        case spv::OpQuantizeToF16:
        case spv::OpConvertPtrToU:
        case spv::OpConvertUToPtr:
        case spv::OpBitcast:
        case spv::OpSNegate:
        case spv::OpFNegate:
        case spv::OpIAdd:
        case spv::OpFAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpVectorTimesScalar:
        case spv::OpMatrixTimesScalar:
        case spv::OpVectorTimesMatrix:
        case spv::OpMatrixTimesVector:
        case spv::OpMatrixTimesMatrix:
        case spv::OpOuterProduct:
        case spv::OpDot:
        case spv::OpIAddCarry:
        case spv::OpISubBorrow:
        case spv::OpUMulExtended:
        case spv::OpSMulExtended:
        case spv::OpAny:
        case spv::OpAll:
        case spv::OpIsNan:
        case spv::OpIsInf:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpLogicalNot:
        case spv::OpSelect:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
        case spv::OpNot:
        case spv::OpBitFieldInsert:
        case spv::OpBitFieldSExtract:
        case spv::OpBitFieldUExtract:
        case spv::OpBitReverse:
        case spv::OpBitCount:
        case spv::OpDPdx:
        case spv::OpDPdy:
        case spv::OpFwidth:
        case spv::OpDPdxFine:
        case spv::OpDPdyFine:
        case spv::OpFwidthFine:
        case spv::OpDPdxCoarse:
        case spv::OpDPdyCoarse:
        case spv::OpFwidthCoarse:
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpPhi:
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupBroadcast:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageSparseTexelsResident:
        case spv::OpImageSparseRead:
        case spv::OpSizeOf:
        case spv::OpGroupNonUniformElect:
        case spv::OpGroupNonUniformAll:
        case spv::OpGroupNonUniformAny:
        case spv::OpGroupNonUniformAllEqual:
        case spv::OpGroupNonUniformBroadcast:
        case spv::OpGroupNonUniformBroadcastFirst:
        case spv::OpGroupNonUniformBallot:
        case spv::OpGroupNonUniformInverseBallot:
        case spv::OpGroupNonUniformBallotBitExtract:
        case spv::OpGroupNonUniformBallotBitCount:
        case spv::OpGroupNonUniformBallotFindLSB:
        case spv::OpGroupNonUniformBallotFindMSB:
        case spv::OpGroupNonUniformShuffle:
        case spv::OpGroupNonUniformShuffleXor:
        case spv::OpGroupNonUniformShuffleUp:
        case spv::OpGroupNonUniformShuffleDown:
        case spv::OpGroupNonUniformIAdd:
        case spv::OpGroupNonUniformFAdd:
        case spv::OpGroupNonUniformIMul:
        case spv::OpGroupNonUniformFMul:
        case spv::OpGroupNonUniformSMin:
        case spv::OpGroupNonUniformUMin:
        case spv::OpGroupNonUniformFMin:
        case spv::OpGroupNonUniformSMax:
        case spv::OpGroupNonUniformUMax:
        case spv::OpGroupNonUniformFMax:
        case spv::OpGroupNonUniformBitwiseAnd:
        case spv::OpGroupNonUniformBitwiseOr:
        case spv::OpGroupNonUniformBitwiseXor:
        case spv::OpGroupNonUniformLogicalAnd:
        case spv::OpGroupNonUniformLogicalOr:
        case spv::OpGroupNonUniformLogicalXor:
        case spv::OpGroupNonUniformQuadBroadcast:
        case spv::OpGroupNonUniformQuadSwap:
        case spv::OpCopyLogical:
        case spv::OpPtrEqual:
        case spv::OpPtrNotEqual:
        case spv::OpPtrDiff:
        case spv::OpColorAttachmentReadEXT:
        case spv::OpDepthAttachmentReadEXT:
        case spv::OpStencilAttachmentReadEXT:
        case spv::OpSubgroupBallotKHR:
        case spv::OpSubgroupFirstInvocationKHR:
        case spv::OpSubgroupAllKHR:
        case spv::OpSubgroupAnyKHR:
        case spv::OpSubgroupAllEqualKHR:
        case spv::OpGroupNonUniformRotateKHR:
        case spv::OpSubgroupReadInvocationKHR:
        case spv::OpExtInstWithForwardRefsKHR:
        case spv::OpConvertUToAccelerationStructureKHR:
        case spv::OpSDot:
        case spv::OpUDot:
        case spv::OpSUDot:
        case spv::OpSDotAccSat:
        case spv::OpUDotAccSat:
        case spv::OpSUDotAccSat:
        case spv::OpCooperativeMatrixLoadKHR:
        case spv::OpCooperativeMatrixMulAddKHR:
        case spv::OpCooperativeMatrixLengthKHR:
        case spv::OpConstantCompositeReplicateEXT:
        case spv::OpSpecConstantCompositeReplicateEXT:
        case spv::OpCompositeConstructReplicateEXT:
        case spv::OpRayQueryProceedKHR:
        case spv::OpRayQueryGetIntersectionTypeKHR:
        case spv::OpImageSampleWeightedQCOM:
        case spv::OpImageBoxFilterQCOM:
        case spv::OpImageBlockMatchSSDQCOM:
        case spv::OpImageBlockMatchSADQCOM:
        case spv::OpImageBlockMatchWindowSSDQCOM:
        case spv::OpImageBlockMatchWindowSADQCOM:
        case spv::OpImageBlockMatchGatherSSDQCOM:
        case spv::OpImageBlockMatchGatherSADQCOM:
        case spv::OpGroupIAddNonUniformAMD:
        case spv::OpGroupFAddNonUniformAMD:
        case spv::OpGroupFMinNonUniformAMD:
        case spv::OpGroupUMinNonUniformAMD:
        case spv::OpGroupSMinNonUniformAMD:
        case spv::OpGroupFMaxNonUniformAMD:
        case spv::OpGroupUMaxNonUniformAMD:
        case spv::OpGroupSMaxNonUniformAMD:
        case spv::OpFragmentMaskFetchAMD:
        case spv::OpFragmentFetchAMD:
        case spv::OpReadClockKHR:
        case spv::OpGroupNonUniformQuadAllKHR:
        case spv::OpGroupNonUniformQuadAnyKHR:
        case spv::OpHitObjectGetWorldToObjectNV:
        case spv::OpHitObjectGetObjectToWorldNV:
        case spv::OpHitObjectGetObjectRayDirectionNV:
        case spv::OpHitObjectGetObjectRayOriginNV:
        case spv::OpHitObjectGetShaderRecordBufferHandleNV:
        case spv::OpHitObjectGetShaderBindingTableRecordIndexNV:
        case spv::OpHitObjectGetCurrentTimeNV:
        case spv::OpHitObjectGetHitKindNV:
        case spv::OpHitObjectGetPrimitiveIndexNV:
        case spv::OpHitObjectGetGeometryIndexNV:
        case spv::OpHitObjectGetInstanceIdNV:
        case spv::OpHitObjectGetInstanceCustomIndexNV:
        case spv::OpHitObjectGetWorldRayDirectionNV:
        case spv::OpHitObjectGetWorldRayOriginNV:
        case spv::OpHitObjectGetRayTMaxNV:
        case spv::OpHitObjectGetRayTMinNV:
        case spv::OpHitObjectIsEmptyNV:
        case spv::OpHitObjectIsHitNV:
        case spv::OpHitObjectIsMissNV:
        case spv::OpImageSampleFootprintNV:
        case spv::OpCooperativeVectorMatrixMulNV:
        case spv::OpCooperativeVectorMatrixMulAddNV:
        case spv::OpCooperativeMatrixConvertNV:
        case spv::OpGroupNonUniformPartitionNV:
        case spv::OpFetchMicroTriangleVertexPositionNV:
        case spv::OpFetchMicroTriangleVertexBarycentricNV:
        case spv::OpCooperativeVectorLoadNV:
        case spv::OpReportIntersectionKHR:
        case spv::OpRayQueryGetIntersectionTriangleVertexPositionsKHR:
        case spv::OpRayQueryGetClusterIdNV:
        case spv::OpHitObjectGetClusterIdNV:
        case spv::OpCooperativeMatrixLoadNV:
        case spv::OpCooperativeMatrixMulAddNV:
        case spv::OpCooperativeMatrixLengthNV:
        case spv::OpCooperativeMatrixReduceNV:
        case spv::OpCooperativeMatrixLoadTensorNV:
        case spv::OpCooperativeMatrixPerElementOpNV:
        case spv::OpCreateTensorLayoutNV:
        case spv::OpTensorLayoutSetDimensionNV:
        case spv::OpTensorLayoutSetStrideNV:
        case spv::OpTensorLayoutSliceNV:
        case spv::OpTensorLayoutSetClampValueNV:
        case spv::OpCreateTensorViewNV:
        case spv::OpTensorViewSetDimensionNV:
        case spv::OpTensorViewSetStrideNV:
        case spv::OpIsHelperInvocationEXT:
        case spv::OpTensorViewSetClipNV:
        case spv::OpTensorLayoutSetBlockSizeNV:
        case spv::OpCooperativeMatrixTransposeNV:
        case spv::OpConvertUToImageNV:
        case spv::OpConvertUToSamplerNV:
        case spv::OpConvertImageToUNV:
        case spv::OpConvertSamplerToUNV:
        case spv::OpConvertUToSampledImageNV:
        case spv::OpConvertSampledImageToUNV:
        case spv::OpRawAccessChainNV:
        case spv::OpRayQueryGetIntersectionSpherePositionNV:
        case spv::OpRayQueryGetIntersectionSphereRadiusNV:
        case spv::OpRayQueryGetIntersectionLSSPositionsNV:
        case spv::OpRayQueryGetIntersectionLSSRadiiNV:
        case spv::OpRayQueryGetIntersectionLSSHitValueNV:
        case spv::OpHitObjectGetSpherePositionNV:
        case spv::OpHitObjectGetSphereRadiusNV:
        case spv::OpHitObjectGetLSSPositionsNV:
        case spv::OpHitObjectGetLSSRadiiNV:
        case spv::OpHitObjectIsSphereHitNV:
        case spv::OpHitObjectIsLSSHitNV:
        case spv::OpRayQueryIsSphereHitNV:
        case spv::OpRayQueryIsLSSHitNV:
        case spv::OpUCountLeadingZerosINTEL:
        case spv::OpUCountTrailingZerosINTEL:
        case spv::OpAbsISubINTEL:
        case spv::OpAbsUSubINTEL:
        case spv::OpIAddSatINTEL:
        case spv::OpUAddSatINTEL:
        case spv::OpIAverageINTEL:
        case spv::OpUAverageINTEL:
        case spv::OpIAverageRoundedINTEL:
        case spv::OpUAverageRoundedINTEL:
        case spv::OpISubSatINTEL:
        case spv::OpUSubSatINTEL:
        case spv::OpIMul32x16INTEL:
        case spv::OpUMul32x16INTEL:
        case spv::OpConstantFunctionPointerINTEL:
        case spv::OpFunctionPointerCallINTEL:
        case spv::OpAtomicFMinEXT:
        case spv::OpAtomicFMaxEXT:
        case spv::OpExpectKHR:
        case spv::OpRayQueryGetRayTMinKHR:
        case spv::OpRayQueryGetRayFlagsKHR:
        case spv::OpRayQueryGetIntersectionTKHR:
        case spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR:
        case spv::OpRayQueryGetIntersectionInstanceIdKHR:
        case spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR:
        case spv::OpRayQueryGetIntersectionGeometryIndexKHR:
        case spv::OpRayQueryGetIntersectionPrimitiveIndexKHR:
        case spv::OpRayQueryGetIntersectionBarycentricsKHR:
        case spv::OpRayQueryGetIntersectionFrontFaceKHR:
        case spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR:
        case spv::OpRayQueryGetIntersectionObjectRayDirectionKHR:
        case spv::OpRayQueryGetIntersectionObjectRayOriginKHR:
        case spv::OpRayQueryGetWorldRayDirectionKHR:
        case spv::OpRayQueryGetWorldRayOriginKHR:
        case spv::OpRayQueryGetIntersectionObjectToWorldKHR:
        case spv::OpRayQueryGetIntersectionWorldToObjectKHR:
        case spv::OpAtomicFAddEXT:
        case spv::OpArithmeticFenceEXT:
        case spv::OpSubgroupMatrixMultiplyAccumulateINTEL:
        case spv::OpGroupIMulKHR:
        case spv::OpGroupFMulKHR:
        case spv::OpGroupBitwiseAndKHR:
        case spv::OpGroupBitwiseOrKHR:
        case spv::OpGroupBitwiseXorKHR:
        case spv::OpGroupLogicalAndKHR:
        case spv::OpGroupLogicalOrKHR:
        case spv::OpGroupLogicalXorKHR:
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::OpUntypedVariableKHR:
        case spv::OpUntypedAccessChainKHR:
        case spv::OpUntypedInBoundsAccessChainKHR:
        case spv::OpUntypedPtrAccessChainKHR:
        case spv::OpUntypedInBoundsPtrAccessChainKHR:
        case spv::OpUntypedArrayLengthKHR:
        case spv::OpAllocateNodePayloadsAMDX:
        case spv::OpFinishWritingNodePayloadAMDX:
        case spv::OpNodePayloadArrayLengthAMDX:
        case spv::OpIsNodePayloadValidAMDX:
#endif
            return true;
        default:
            return false;
    }
}

static constexpr bool OpcodeHasResult(uint32_t opcode) {
    switch (opcode) {
        case spv::OpUndef:
        case spv::OpString:
        case spv::OpExtInstImport:
        case spv::OpExtInst:
        case spv::OpTypeVoid:
        case spv::OpTypeBool:
        case spv::OpTypeInt:
        case spv::OpTypeFloat:
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeImage:
        case spv::OpTypeSampler:
        case spv::OpTypeSampledImage:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeStruct:
        case spv::OpTypePointer:
        case spv::OpTypeFunction:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstant:
        case spv::OpConstantComposite:
        case spv::OpConstantNull:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstant:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:
        case spv::OpFunction:
        case spv::OpFunctionParameter:
        case spv::OpFunctionCall:
        case spv::OpVariable:
        case spv::OpImageTexelPointer:
        case spv::OpLoad:
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
        case spv::OpPtrAccessChain:
        case spv::OpArrayLength:
        case spv::OpInBoundsPtrAccessChain:
        case spv::OpDecorationGroup:
        case spv::OpVectorExtractDynamic:
        case spv::OpVectorInsertDynamic:
        case spv::OpVectorShuffle:
        case spv::OpCompositeConstruct:
        case spv::OpCompositeExtract:
        case spv::OpCompositeInsert:
        case spv::OpCopyObject:
        case spv::OpTranspose:
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpConvertFToU:
        case spv::OpConvertFToS:
        case spv::OpConvertSToF:
        case spv::OpConvertUToF:
        case spv::OpUConvert:
        case spv::OpSConvert:
        case spv::OpFConvert:
        case spv::OpQuantizeToF16:
        case spv::OpConvertPtrToU:
        case spv::OpConvertUToPtr:
        case spv::OpBitcast:
        case spv::OpSNegate:
        case spv::OpFNegate:
        case spv::OpIAdd:
        case spv::OpFAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpVectorTimesScalar:
        case spv::OpMatrixTimesScalar:
        case spv::OpVectorTimesMatrix:
        case spv::OpMatrixTimesVector:
        case spv::OpMatrixTimesMatrix:
        case spv::OpOuterProduct:
        case spv::OpDot:
        case spv::OpIAddCarry:
        case spv::OpISubBorrow:
        case spv::OpUMulExtended:
        case spv::OpSMulExtended:
        case spv::OpAny:
        case spv::OpAll:
        case spv::OpIsNan:
        case spv::OpIsInf:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpLogicalNot:
        case spv::OpSelect:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
        case spv::OpNot:
        case spv::OpBitFieldInsert:
        case spv::OpBitFieldSExtract:
        case spv::OpBitFieldUExtract:
        case spv::OpBitReverse:
        case spv::OpBitCount:
        case spv::OpDPdx:
        case spv::OpDPdy:
        case spv::OpFwidth:
        case spv::OpDPdxFine:
        case spv::OpDPdyFine:
        case spv::OpFwidthFine:
        case spv::OpDPdxCoarse:
        case spv::OpDPdyCoarse:
        case spv::OpFwidthCoarse:
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpPhi:
        case spv::OpLabel:
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupBroadcast:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageSparseTexelsResident:
        case spv::OpImageSparseRead:
        case spv::OpSizeOf:
        case spv::OpGroupNonUniformElect:
        case spv::OpGroupNonUniformAll:
        case spv::OpGroupNonUniformAny:
        case spv::OpGroupNonUniformAllEqual:
        case spv::OpGroupNonUniformBroadcast:
        case spv::OpGroupNonUniformBroadcastFirst:
        case spv::OpGroupNonUniformBallot:
        case spv::OpGroupNonUniformInverseBallot:
        case spv::OpGroupNonUniformBallotBitExtract:
        case spv::OpGroupNonUniformBallotBitCount:
        case spv::OpGroupNonUniformBallotFindLSB:
        case spv::OpGroupNonUniformBallotFindMSB:
        case spv::OpGroupNonUniformShuffle:
        case spv::OpGroupNonUniformShuffleXor:
        case spv::OpGroupNonUniformShuffleUp:
        case spv::OpGroupNonUniformShuffleDown:
        case spv::OpGroupNonUniformIAdd:
        case spv::OpGroupNonUniformFAdd:
        case spv::OpGroupNonUniformIMul:
        case spv::OpGroupNonUniformFMul:
        case spv::OpGroupNonUniformSMin:
        case spv::OpGroupNonUniformUMin:
        case spv::OpGroupNonUniformFMin:
        case spv::OpGroupNonUniformSMax:
        case spv::OpGroupNonUniformUMax:
        case spv::OpGroupNonUniformFMax:
        case spv::OpGroupNonUniformBitwiseAnd:
        case spv::OpGroupNonUniformBitwiseOr:
        case spv::OpGroupNonUniformBitwiseXor:
        case spv::OpGroupNonUniformLogicalAnd:
        case spv::OpGroupNonUniformLogicalOr:
        case spv::OpGroupNonUniformLogicalXor:
        case spv::OpGroupNonUniformQuadBroadcast:
        case spv::OpGroupNonUniformQuadSwap:
        case spv::OpCopyLogical:
        case spv::OpPtrEqual:
        case spv::OpPtrNotEqual:
        case spv::OpPtrDiff:
        case spv::OpColorAttachmentReadEXT:
        case spv::OpDepthAttachmentReadEXT:
        case spv::OpStencilAttachmentReadEXT:
        case spv::OpSubgroupBallotKHR:
        case spv::OpSubgroupFirstInvocationKHR:
        case spv::OpSubgroupAllKHR:
        case spv::OpSubgroupAnyKHR:
        case spv::OpSubgroupAllEqualKHR:
        case spv::OpGroupNonUniformRotateKHR:
        case spv::OpSubgroupReadInvocationKHR:
        case spv::OpExtInstWithForwardRefsKHR:
        case spv::OpConvertUToAccelerationStructureKHR:
        case spv::OpSDot:
        case spv::OpUDot:
        case spv::OpSUDot:
        case spv::OpSDotAccSat:
        case spv::OpUDotAccSat:
        case spv::OpSUDotAccSat:
        case spv::OpTypeCooperativeMatrixKHR:
        case spv::OpCooperativeMatrixLoadKHR:
        case spv::OpCooperativeMatrixMulAddKHR:
        case spv::OpCooperativeMatrixLengthKHR:
        case spv::OpConstantCompositeReplicateEXT:
        case spv::OpSpecConstantCompositeReplicateEXT:
        case spv::OpCompositeConstructReplicateEXT:
        case spv::OpTypeRayQueryKHR:
        case spv::OpRayQueryProceedKHR:
        case spv::OpRayQueryGetIntersectionTypeKHR:
        case spv::OpImageSampleWeightedQCOM:
        case spv::OpImageBoxFilterQCOM:
        case spv::OpImageBlockMatchSSDQCOM:
        case spv::OpImageBlockMatchSADQCOM:
        case spv::OpImageBlockMatchWindowSSDQCOM:
        case spv::OpImageBlockMatchWindowSADQCOM:
        case spv::OpImageBlockMatchGatherSSDQCOM:
        case spv::OpImageBlockMatchGatherSADQCOM:
        case spv::OpGroupIAddNonUniformAMD:
        case spv::OpGroupFAddNonUniformAMD:
        case spv::OpGroupFMinNonUniformAMD:
        case spv::OpGroupUMinNonUniformAMD:
        case spv::OpGroupSMinNonUniformAMD:
        case spv::OpGroupFMaxNonUniformAMD:
        case spv::OpGroupUMaxNonUniformAMD:
        case spv::OpGroupSMaxNonUniformAMD:
        case spv::OpFragmentMaskFetchAMD:
        case spv::OpFragmentFetchAMD:
        case spv::OpReadClockKHR:
        case spv::OpGroupNonUniformQuadAllKHR:
        case spv::OpGroupNonUniformQuadAnyKHR:
        case spv::OpHitObjectGetWorldToObjectNV:
        case spv::OpHitObjectGetObjectToWorldNV:
        case spv::OpHitObjectGetObjectRayDirectionNV:
        case spv::OpHitObjectGetObjectRayOriginNV:
        case spv::OpHitObjectGetShaderRecordBufferHandleNV:
        case spv::OpHitObjectGetShaderBindingTableRecordIndexNV:
        case spv::OpHitObjectGetCurrentTimeNV:
        case spv::OpHitObjectGetHitKindNV:
        case spv::OpHitObjectGetPrimitiveIndexNV:
        case spv::OpHitObjectGetGeometryIndexNV:
        case spv::OpHitObjectGetInstanceIdNV:
        case spv::OpHitObjectGetInstanceCustomIndexNV:
        case spv::OpHitObjectGetWorldRayDirectionNV:
        case spv::OpHitObjectGetWorldRayOriginNV:
        case spv::OpHitObjectGetRayTMaxNV:
        case spv::OpHitObjectGetRayTMinNV:
        case spv::OpHitObjectIsEmptyNV:
        case spv::OpHitObjectIsHitNV:
        case spv::OpHitObjectIsMissNV:
        case spv::OpTypeHitObjectNV:
        case spv::OpImageSampleFootprintNV:
        case spv::OpTypeCooperativeVectorNV:
        case spv::OpCooperativeVectorMatrixMulNV:
        case spv::OpCooperativeVectorMatrixMulAddNV:
        case spv::OpCooperativeMatrixConvertNV:
        case spv::OpGroupNonUniformPartitionNV:
        case spv::OpFetchMicroTriangleVertexPositionNV:
        case spv::OpFetchMicroTriangleVertexBarycentricNV:
        case spv::OpCooperativeVectorLoadNV:
        case spv::OpReportIntersectionKHR:
        case spv::OpRayQueryGetIntersectionTriangleVertexPositionsKHR:
        case spv::OpTypeAccelerationStructureKHR:
        case spv::OpRayQueryGetClusterIdNV:
        case spv::OpHitObjectGetClusterIdNV:
        case spv::OpTypeCooperativeMatrixNV:
        case spv::OpCooperativeMatrixLoadNV:
        case spv::OpCooperativeMatrixMulAddNV:
        case spv::OpCooperativeMatrixLengthNV:
        case spv::OpCooperativeMatrixReduceNV:
        case spv::OpCooperativeMatrixLoadTensorNV:
        case spv::OpCooperativeMatrixPerElementOpNV:
        case spv::OpTypeTensorLayoutNV:
        case spv::OpTypeTensorViewNV:
        case spv::OpCreateTensorLayoutNV:
        case spv::OpTensorLayoutSetDimensionNV:
        case spv::OpTensorLayoutSetStrideNV:
        case spv::OpTensorLayoutSliceNV:
        case spv::OpTensorLayoutSetClampValueNV:
        case spv::OpCreateTensorViewNV:
        case spv::OpTensorViewSetDimensionNV:
        case spv::OpTensorViewSetStrideNV:
        case spv::OpIsHelperInvocationEXT:
        case spv::OpTensorViewSetClipNV:
        case spv::OpTensorLayoutSetBlockSizeNV:
        case spv::OpCooperativeMatrixTransposeNV:
        case spv::OpConvertUToImageNV:
        case spv::OpConvertUToSamplerNV:
        case spv::OpConvertImageToUNV:
        case spv::OpConvertSamplerToUNV:
        case spv::OpConvertUToSampledImageNV:
        case spv::OpConvertSampledImageToUNV:
        case spv::OpRawAccessChainNV:
        case spv::OpRayQueryGetIntersectionSpherePositionNV:
        case spv::OpRayQueryGetIntersectionSphereRadiusNV:
        case spv::OpRayQueryGetIntersectionLSSPositionsNV:
        case spv::OpRayQueryGetIntersectionLSSRadiiNV:
        case spv::OpRayQueryGetIntersectionLSSHitValueNV:
        case spv::OpHitObjectGetSpherePositionNV:
        case spv::OpHitObjectGetSphereRadiusNV:
        case spv::OpHitObjectGetLSSPositionsNV:
        case spv::OpHitObjectGetLSSRadiiNV:
        case spv::OpHitObjectIsSphereHitNV:
        case spv::OpHitObjectIsLSSHitNV:
        case spv::OpRayQueryIsSphereHitNV:
        case spv::OpRayQueryIsLSSHitNV:
        case spv::OpUCountLeadingZerosINTEL:
        case spv::OpUCountTrailingZerosINTEL:
        case spv::OpAbsISubINTEL:
        case spv::OpAbsUSubINTEL:
        case spv::OpIAddSatINTEL:
        case spv::OpUAddSatINTEL:
        case spv::OpIAverageINTEL:
        case spv::OpUAverageINTEL:
        case spv::OpIAverageRoundedINTEL:
        case spv::OpUAverageRoundedINTEL:
        case spv::OpISubSatINTEL:
        case spv::OpUSubSatINTEL:
        case spv::OpIMul32x16INTEL:
        case spv::OpUMul32x16INTEL:
        case spv::OpConstantFunctionPointerINTEL:
        case spv::OpFunctionPointerCallINTEL:
        case spv::OpAtomicFMinEXT:
        case spv::OpAtomicFMaxEXT:
        case spv::OpExpectKHR:
        case spv::OpRayQueryGetRayTMinKHR:
        case spv::OpRayQueryGetRayFlagsKHR:
        case spv::OpRayQueryGetIntersectionTKHR:
        case spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR:
        case spv::OpRayQueryGetIntersectionInstanceIdKHR:
        case spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR:
        case spv::OpRayQueryGetIntersectionGeometryIndexKHR:
        case spv::OpRayQueryGetIntersectionPrimitiveIndexKHR:
        case spv::OpRayQueryGetIntersectionBarycentricsKHR:
        case spv::OpRayQueryGetIntersectionFrontFaceKHR:
        case spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR:
        case spv::OpRayQueryGetIntersectionObjectRayDirectionKHR:
        case spv::OpRayQueryGetIntersectionObjectRayOriginKHR:
        case spv::OpRayQueryGetWorldRayDirectionKHR:
        case spv::OpRayQueryGetWorldRayOriginKHR:
        case spv::OpRayQueryGetIntersectionObjectToWorldKHR:
        case spv::OpRayQueryGetIntersectionWorldToObjectKHR:
        case spv::OpAtomicFAddEXT:
        case spv::OpArithmeticFenceEXT:
        case spv::OpSubgroupMatrixMultiplyAccumulateINTEL:
        case spv::OpGroupIMulKHR:
        case spv::OpGroupFMulKHR:
        case spv::OpGroupBitwiseAndKHR:
        case spv::OpGroupBitwiseOrKHR:
        case spv::OpGroupBitwiseXorKHR:
        case spv::OpGroupLogicalAndKHR:
        case spv::OpGroupLogicalOrKHR:
        case spv::OpGroupLogicalXorKHR:
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::OpTypeUntypedPointerKHR:
        case spv::OpUntypedVariableKHR:
        case spv::OpUntypedAccessChainKHR:
        case spv::OpUntypedInBoundsAccessChainKHR:
        case spv::OpUntypedPtrAccessChainKHR:
        case spv::OpUntypedInBoundsPtrAccessChainKHR:
        case spv::OpUntypedArrayLengthKHR:
        case spv::OpAllocateNodePayloadsAMDX:
        case spv::OpTypeNodePayloadArrayAMDX:
        case spv::OpFinishWritingNodePayloadAMDX:
        case spv::OpNodePayloadArrayLengthAMDX:
        case spv::OpIsNodePayloadValidAMDX:
        case spv::OpConstantStringAMDX:
        case spv::OpSpecConstantStringAMDX:
#endif
            return true;
        default:
            return false;
    }
}

// Any non supported operation will be covered with other VUs
static constexpr bool AtomicOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpAtomicLoad:
        case spv::OpAtomicStore:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFMinEXT:
        case spv::OpAtomicFMaxEXT:
        case spv::OpAtomicFAddEXT:
            return true;
        default:
            return false;
    }
}

// Any non supported operation will be covered with other VUs
static constexpr bool GroupOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpGroupNonUniformElect:
        case spv::OpGroupNonUniformAll:
        case spv::OpGroupNonUniformAny:
        case spv::OpGroupNonUniformAllEqual:
        case spv::OpGroupNonUniformBroadcast:
        case spv::OpGroupNonUniformBroadcastFirst:
        case spv::OpGroupNonUniformBallot:
        case spv::OpGroupNonUniformInverseBallot:
        case spv::OpGroupNonUniformBallotBitExtract:
        case spv::OpGroupNonUniformBallotBitCount:
        case spv::OpGroupNonUniformBallotFindLSB:
        case spv::OpGroupNonUniformBallotFindMSB:
        case spv::OpGroupNonUniformShuffle:
        case spv::OpGroupNonUniformShuffleXor:
        case spv::OpGroupNonUniformShuffleUp:
        case spv::OpGroupNonUniformShuffleDown:
        case spv::OpGroupNonUniformIAdd:
        case spv::OpGroupNonUniformFAdd:
        case spv::OpGroupNonUniformIMul:
        case spv::OpGroupNonUniformFMul:
        case spv::OpGroupNonUniformSMin:
        case spv::OpGroupNonUniformUMin:
        case spv::OpGroupNonUniformFMin:
        case spv::OpGroupNonUniformSMax:
        case spv::OpGroupNonUniformUMax:
        case spv::OpGroupNonUniformFMax:
        case spv::OpGroupNonUniformBitwiseAnd:
        case spv::OpGroupNonUniformBitwiseOr:
        case spv::OpGroupNonUniformBitwiseXor:
        case spv::OpGroupNonUniformLogicalAnd:
        case spv::OpGroupNonUniformLogicalOr:
        case spv::OpGroupNonUniformLogicalXor:
        case spv::OpGroupNonUniformQuadBroadcast:
        case spv::OpGroupNonUniformQuadSwap:
        case spv::OpGroupNonUniformQuadAllKHR:
        case spv::OpGroupNonUniformQuadAnyKHR:
        case spv::OpGroupNonUniformPartitionNV:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageGatherOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageBlockMatchGatherSSDQCOM:
        case spv::OpImageBlockMatchGatherSADQCOM:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageFetchOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageFetch:
            return true;
        default:
            return false;
    }
}

static constexpr bool ImageSampleOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageSampleWeightedQCOM:
        case spv::OpImageSampleFootprintNV:
            return true;
        default:
            return false;
    }
}

// Return number of optional parameter from ImageOperands
static constexpr uint32_t ImageOperandsParamCount(uint32_t image_operand) {
    uint32_t count = 0;
    switch (image_operand) {
        case spv::ImageOperandsMaskNone:
        case spv::ImageOperandsNonPrivateTexelMask:
        case spv::ImageOperandsVolatileTexelMask:
        case spv::ImageOperandsSignExtendMask:
        case spv::ImageOperandsZeroExtendMask:
        case spv::ImageOperandsNontemporalMask:
            return 0;
        case spv::ImageOperandsBiasMask:
        case spv::ImageOperandsLodMask:
        case spv::ImageOperandsConstOffsetMask:
        case spv::ImageOperandsOffsetMask:
        case spv::ImageOperandsConstOffsetsMask:
        case spv::ImageOperandsSampleMask:
        case spv::ImageOperandsMinLodMask:
        case spv::ImageOperandsMakeTexelAvailableMask:
        case spv::ImageOperandsMakeTexelVisibleMask:
        case spv::ImageOperandsOffsetsMask:
            return 1;
        case spv::ImageOperandsGradMask:
            return 2;

        default:
            break;
    }
    return count;
}

// Return operand position of Memory Scope <ID> or zero if there is none
static constexpr uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpMemoryBarrier:
            return 1;
        case spv::OpControlBarrier:
        case spv::OpAtomicStore:
            return 2;
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFMinEXT:
        case spv::OpAtomicFMaxEXT:
        case spv::OpAtomicFAddEXT:
            return 4;

        default:
            break;
    }
    return position;
}

// Return operand position of Execution Scope <ID> or zero if there is none
static constexpr uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpControlBarrier:
            return 1;
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupBroadcast:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
        case spv::OpGroupNonUniformElect:
        case spv::OpGroupNonUniformAll:
        case spv::OpGroupNonUniformAny:
        case spv::OpGroupNonUniformAllEqual:
        case spv::OpGroupNonUniformBroadcast:
        case spv::OpGroupNonUniformBroadcastFirst:
        case spv::OpGroupNonUniformBallot:
        case spv::OpGroupNonUniformInverseBallot:
        case spv::OpGroupNonUniformBallotBitExtract:
        case spv::OpGroupNonUniformBallotBitCount:
        case spv::OpGroupNonUniformBallotFindLSB:
        case spv::OpGroupNonUniformBallotFindMSB:
        case spv::OpGroupNonUniformShuffle:
        case spv::OpGroupNonUniformShuffleXor:
        case spv::OpGroupNonUniformShuffleUp:
        case spv::OpGroupNonUniformShuffleDown:
        case spv::OpGroupNonUniformIAdd:
        case spv::OpGroupNonUniformFAdd:
        case spv::OpGroupNonUniformIMul:
        case spv::OpGroupNonUniformFMul:
        case spv::OpGroupNonUniformSMin:
        case spv::OpGroupNonUniformUMin:
        case spv::OpGroupNonUniformFMin:
        case spv::OpGroupNonUniformSMax:
        case spv::OpGroupNonUniformUMax:
        case spv::OpGroupNonUniformFMax:
        case spv::OpGroupNonUniformBitwiseAnd:
        case spv::OpGroupNonUniformBitwiseOr:
        case spv::OpGroupNonUniformBitwiseXor:
        case spv::OpGroupNonUniformLogicalAnd:
        case spv::OpGroupNonUniformLogicalOr:
        case spv::OpGroupNonUniformLogicalXor:
        case spv::OpGroupNonUniformQuadBroadcast:
        case spv::OpGroupNonUniformQuadSwap:
        case spv::OpGroupNonUniformRotateKHR:
        case spv::OpTypeCooperativeMatrixKHR:
        case spv::OpGroupIAddNonUniformAMD:
        case spv::OpGroupFAddNonUniformAMD:
        case spv::OpGroupFMinNonUniformAMD:
        case spv::OpGroupUMinNonUniformAMD:
        case spv::OpGroupSMinNonUniformAMD:
        case spv::OpGroupFMaxNonUniformAMD:
        case spv::OpGroupUMaxNonUniformAMD:
        case spv::OpGroupSMaxNonUniformAMD:
        case spv::OpReadClockKHR:
        case spv::OpTypeCooperativeMatrixNV:
        case spv::OpGroupIMulKHR:
        case spv::OpGroupFMulKHR:
        case spv::OpGroupBitwiseAndKHR:
        case spv::OpGroupBitwiseOrKHR:
        case spv::OpGroupBitwiseXorKHR:
        case spv::OpGroupLogicalAndKHR:
        case spv::OpGroupLogicalOrKHR:
        case spv::OpGroupLogicalXorKHR:
            return 3;

        default:
            break;
    }
    return position;
}

// Return operand position of Image Operands <ID> or zero if there is none
static constexpr uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpImageWrite:
            return 4;
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageRead:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseRead:
            return 5;
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
            return 6;
        case spv::OpImageSampleFootprintNV:
            return 7;

        default:
            break;
    }
    return position;
}

// Return operand position of 'Image' or 'Sampled Image' IdRef or zero if there is none.
static constexpr uint32_t OpcodeImageAccessPosition(uint32_t opcode) {
    uint32_t position = 0;
    switch (opcode) {
        case spv::OpImageWrite:
            return 1;
        case spv::OpImageTexelPointer:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageSparseRead:
        case spv::OpFragmentMaskFetchAMD:
        case spv::OpFragmentFetchAMD:
        case spv::OpImageSampleFootprintNV:
            return 3;

        default:
            break;
    }
    return position;
}

// All valid OpType*
enum class SpvType {
    Empty = 0,
    kVoid,
    kBool,
    kInt,
    kFloat,
    kVector,
    kMatrix,
    kImage,
    kSampler,
    kSampledImage,
    kArray,
    kRuntimeArray,
    kStruct,
    kPointer,
    kFunction,
    kForwardPointer,
    kCooperativeMatrixKHR,
    kRayQueryKHR,
    kHitObjectNV,
    kCooperativeVectorNV,
    kAccelerationStructureKHR,
    kCooperativeMatrixNV,
    kTensorLayoutNV,
    kTensorViewNV,
};

static constexpr SpvType GetSpvType(uint32_t opcode) {
    switch (opcode) {
        case spv::OpTypeVoid:
            return SpvType::kVoid;
        case spv::OpTypeBool:
            return SpvType::kBool;
        case spv::OpTypeInt:
            return SpvType::kInt;
        case spv::OpTypeFloat:
            return SpvType::kFloat;
        case spv::OpTypeVector:
            return SpvType::kVector;
        case spv::OpTypeMatrix:
            return SpvType::kMatrix;
        case spv::OpTypeImage:
            return SpvType::kImage;
        case spv::OpTypeSampler:
            return SpvType::kSampler;
        case spv::OpTypeSampledImage:
            return SpvType::kSampledImage;
        case spv::OpTypeArray:
            return SpvType::kArray;
        case spv::OpTypeRuntimeArray:
            return SpvType::kRuntimeArray;
        case spv::OpTypeStruct:
            return SpvType::kStruct;
        case spv::OpTypePointer:
            return SpvType::kPointer;
        case spv::OpTypeFunction:
            return SpvType::kFunction;
        case spv::OpTypeForwardPointer:
            return SpvType::kForwardPointer;
        case spv::OpTypeCooperativeMatrixKHR:
            return SpvType::kCooperativeMatrixKHR;
        case spv::OpTypeRayQueryKHR:
            return SpvType::kRayQueryKHR;
        case spv::OpTypeHitObjectNV:
            return SpvType::kHitObjectNV;
        case spv::OpTypeCooperativeVectorNV:
            return SpvType::kCooperativeVectorNV;
        case spv::OpTypeAccelerationStructureKHR:
            return SpvType::kAccelerationStructureKHR;
        case spv::OpTypeCooperativeMatrixNV:
            return SpvType::kCooperativeMatrixNV;
        case spv::OpTypeTensorLayoutNV:
            return SpvType::kTensorLayoutNV;
        case spv::OpTypeTensorViewNV:
            return SpvType::kTensorViewNV;
        default:
            return SpvType::Empty;
    }
}

enum class OperandKind {
    Invalid = 0,
    Id,
    Label,  // Id but for Control Flow
    Literal,
    LiteralString,
    Composite,
    ValueEnum,
    BitEnum,
};

struct OperandInfo {
    std::vector<OperandKind> types;
};

const OperandInfo& GetOperandInfo(uint32_t opcode);

// NOLINTEND
