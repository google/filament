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

#include "containers/custom_containers.h"
#include "spirv_grammar_helper.h"

const char* string_SpvOpcode(uint32_t opcode) {
    switch (opcode) {
        case spv::OpNop:
            return "OpNop";
        case spv::OpUndef:
            return "OpUndef";
        case spv::OpSourceContinued:
            return "OpSourceContinued";
        case spv::OpSource:
            return "OpSource";
        case spv::OpSourceExtension:
            return "OpSourceExtension";
        case spv::OpName:
            return "OpName";
        case spv::OpMemberName:
            return "OpMemberName";
        case spv::OpString:
            return "OpString";
        case spv::OpLine:
            return "OpLine";
        case spv::OpExtension:
            return "OpExtension";
        case spv::OpExtInstImport:
            return "OpExtInstImport";
        case spv::OpExtInst:
            return "OpExtInst";
        case spv::OpMemoryModel:
            return "OpMemoryModel";
        case spv::OpEntryPoint:
            return "OpEntryPoint";
        case spv::OpExecutionMode:
            return "OpExecutionMode";
        case spv::OpCapability:
            return "OpCapability";
        case spv::OpTypeVoid:
            return "OpTypeVoid";
        case spv::OpTypeBool:
            return "OpTypeBool";
        case spv::OpTypeInt:
            return "OpTypeInt";
        case spv::OpTypeFloat:
            return "OpTypeFloat";
        case spv::OpTypeVector:
            return "OpTypeVector";
        case spv::OpTypeMatrix:
            return "OpTypeMatrix";
        case spv::OpTypeImage:
            return "OpTypeImage";
        case spv::OpTypeSampler:
            return "OpTypeSampler";
        case spv::OpTypeSampledImage:
            return "OpTypeSampledImage";
        case spv::OpTypeArray:
            return "OpTypeArray";
        case spv::OpTypeRuntimeArray:
            return "OpTypeRuntimeArray";
        case spv::OpTypeStruct:
            return "OpTypeStruct";
        case spv::OpTypePointer:
            return "OpTypePointer";
        case spv::OpTypeFunction:
            return "OpTypeFunction";
        case spv::OpTypeForwardPointer:
            return "OpTypeForwardPointer";
        case spv::OpConstantTrue:
            return "OpConstantTrue";
        case spv::OpConstantFalse:
            return "OpConstantFalse";
        case spv::OpConstant:
            return "OpConstant";
        case spv::OpConstantComposite:
            return "OpConstantComposite";
        case spv::OpConstantNull:
            return "OpConstantNull";
        case spv::OpSpecConstantTrue:
            return "OpSpecConstantTrue";
        case spv::OpSpecConstantFalse:
            return "OpSpecConstantFalse";
        case spv::OpSpecConstant:
            return "OpSpecConstant";
        case spv::OpSpecConstantComposite:
            return "OpSpecConstantComposite";
        case spv::OpSpecConstantOp:
            return "OpSpecConstantOp";
        case spv::OpFunction:
            return "OpFunction";
        case spv::OpFunctionParameter:
            return "OpFunctionParameter";
        case spv::OpFunctionEnd:
            return "OpFunctionEnd";
        case spv::OpFunctionCall:
            return "OpFunctionCall";
        case spv::OpVariable:
            return "OpVariable";
        case spv::OpImageTexelPointer:
            return "OpImageTexelPointer";
        case spv::OpLoad:
            return "OpLoad";
        case spv::OpStore:
            return "OpStore";
        case spv::OpCopyMemory:
            return "OpCopyMemory";
        case spv::OpCopyMemorySized:
            return "OpCopyMemorySized";
        case spv::OpAccessChain:
            return "OpAccessChain";
        case spv::OpInBoundsAccessChain:
            return "OpInBoundsAccessChain";
        case spv::OpPtrAccessChain:
            return "OpPtrAccessChain";
        case spv::OpArrayLength:
            return "OpArrayLength";
        case spv::OpInBoundsPtrAccessChain:
            return "OpInBoundsPtrAccessChain";
        case spv::OpDecorate:
            return "OpDecorate";
        case spv::OpMemberDecorate:
            return "OpMemberDecorate";
        case spv::OpDecorationGroup:
            return "OpDecorationGroup";
        case spv::OpGroupDecorate:
            return "OpGroupDecorate";
        case spv::OpGroupMemberDecorate:
            return "OpGroupMemberDecorate";
        case spv::OpVectorExtractDynamic:
            return "OpVectorExtractDynamic";
        case spv::OpVectorInsertDynamic:
            return "OpVectorInsertDynamic";
        case spv::OpVectorShuffle:
            return "OpVectorShuffle";
        case spv::OpCompositeConstruct:
            return "OpCompositeConstruct";
        case spv::OpCompositeExtract:
            return "OpCompositeExtract";
        case spv::OpCompositeInsert:
            return "OpCompositeInsert";
        case spv::OpCopyObject:
            return "OpCopyObject";
        case spv::OpTranspose:
            return "OpTranspose";
        case spv::OpSampledImage:
            return "OpSampledImage";
        case spv::OpImageSampleImplicitLod:
            return "OpImageSampleImplicitLod";
        case spv::OpImageSampleExplicitLod:
            return "OpImageSampleExplicitLod";
        case spv::OpImageSampleDrefImplicitLod:
            return "OpImageSampleDrefImplicitLod";
        case spv::OpImageSampleDrefExplicitLod:
            return "OpImageSampleDrefExplicitLod";
        case spv::OpImageSampleProjImplicitLod:
            return "OpImageSampleProjImplicitLod";
        case spv::OpImageSampleProjExplicitLod:
            return "OpImageSampleProjExplicitLod";
        case spv::OpImageSampleProjDrefImplicitLod:
            return "OpImageSampleProjDrefImplicitLod";
        case spv::OpImageSampleProjDrefExplicitLod:
            return "OpImageSampleProjDrefExplicitLod";
        case spv::OpImageFetch:
            return "OpImageFetch";
        case spv::OpImageGather:
            return "OpImageGather";
        case spv::OpImageDrefGather:
            return "OpImageDrefGather";
        case spv::OpImageRead:
            return "OpImageRead";
        case spv::OpImageWrite:
            return "OpImageWrite";
        case spv::OpImage:
            return "OpImage";
        case spv::OpImageQuerySizeLod:
            return "OpImageQuerySizeLod";
        case spv::OpImageQuerySize:
            return "OpImageQuerySize";
        case spv::OpImageQueryLod:
            return "OpImageQueryLod";
        case spv::OpImageQueryLevels:
            return "OpImageQueryLevels";
        case spv::OpImageQuerySamples:
            return "OpImageQuerySamples";
        case spv::OpConvertFToU:
            return "OpConvertFToU";
        case spv::OpConvertFToS:
            return "OpConvertFToS";
        case spv::OpConvertSToF:
            return "OpConvertSToF";
        case spv::OpConvertUToF:
            return "OpConvertUToF";
        case spv::OpUConvert:
            return "OpUConvert";
        case spv::OpSConvert:
            return "OpSConvert";
        case spv::OpFConvert:
            return "OpFConvert";
        case spv::OpQuantizeToF16:
            return "OpQuantizeToF16";
        case spv::OpConvertPtrToU:
            return "OpConvertPtrToU";
        case spv::OpConvertUToPtr:
            return "OpConvertUToPtr";
        case spv::OpBitcast:
            return "OpBitcast";
        case spv::OpSNegate:
            return "OpSNegate";
        case spv::OpFNegate:
            return "OpFNegate";
        case spv::OpIAdd:
            return "OpIAdd";
        case spv::OpFAdd:
            return "OpFAdd";
        case spv::OpISub:
            return "OpISub";
        case spv::OpFSub:
            return "OpFSub";
        case spv::OpIMul:
            return "OpIMul";
        case spv::OpFMul:
            return "OpFMul";
        case spv::OpUDiv:
            return "OpUDiv";
        case spv::OpSDiv:
            return "OpSDiv";
        case spv::OpFDiv:
            return "OpFDiv";
        case spv::OpUMod:
            return "OpUMod";
        case spv::OpSRem:
            return "OpSRem";
        case spv::OpSMod:
            return "OpSMod";
        case spv::OpFRem:
            return "OpFRem";
        case spv::OpFMod:
            return "OpFMod";
        case spv::OpVectorTimesScalar:
            return "OpVectorTimesScalar";
        case spv::OpMatrixTimesScalar:
            return "OpMatrixTimesScalar";
        case spv::OpVectorTimesMatrix:
            return "OpVectorTimesMatrix";
        case spv::OpMatrixTimesVector:
            return "OpMatrixTimesVector";
        case spv::OpMatrixTimesMatrix:
            return "OpMatrixTimesMatrix";
        case spv::OpOuterProduct:
            return "OpOuterProduct";
        case spv::OpDot:
            return "OpDot";
        case spv::OpIAddCarry:
            return "OpIAddCarry";
        case spv::OpISubBorrow:
            return "OpISubBorrow";
        case spv::OpUMulExtended:
            return "OpUMulExtended";
        case spv::OpSMulExtended:
            return "OpSMulExtended";
        case spv::OpAny:
            return "OpAny";
        case spv::OpAll:
            return "OpAll";
        case spv::OpIsNan:
            return "OpIsNan";
        case spv::OpIsInf:
            return "OpIsInf";
        case spv::OpLogicalEqual:
            return "OpLogicalEqual";
        case spv::OpLogicalNotEqual:
            return "OpLogicalNotEqual";
        case spv::OpLogicalOr:
            return "OpLogicalOr";
        case spv::OpLogicalAnd:
            return "OpLogicalAnd";
        case spv::OpLogicalNot:
            return "OpLogicalNot";
        case spv::OpSelect:
            return "OpSelect";
        case spv::OpIEqual:
            return "OpIEqual";
        case spv::OpINotEqual:
            return "OpINotEqual";
        case spv::OpUGreaterThan:
            return "OpUGreaterThan";
        case spv::OpSGreaterThan:
            return "OpSGreaterThan";
        case spv::OpUGreaterThanEqual:
            return "OpUGreaterThanEqual";
        case spv::OpSGreaterThanEqual:
            return "OpSGreaterThanEqual";
        case spv::OpULessThan:
            return "OpULessThan";
        case spv::OpSLessThan:
            return "OpSLessThan";
        case spv::OpULessThanEqual:
            return "OpULessThanEqual";
        case spv::OpSLessThanEqual:
            return "OpSLessThanEqual";
        case spv::OpFOrdEqual:
            return "OpFOrdEqual";
        case spv::OpFUnordEqual:
            return "OpFUnordEqual";
        case spv::OpFOrdNotEqual:
            return "OpFOrdNotEqual";
        case spv::OpFUnordNotEqual:
            return "OpFUnordNotEqual";
        case spv::OpFOrdLessThan:
            return "OpFOrdLessThan";
        case spv::OpFUnordLessThan:
            return "OpFUnordLessThan";
        case spv::OpFOrdGreaterThan:
            return "OpFOrdGreaterThan";
        case spv::OpFUnordGreaterThan:
            return "OpFUnordGreaterThan";
        case spv::OpFOrdLessThanEqual:
            return "OpFOrdLessThanEqual";
        case spv::OpFUnordLessThanEqual:
            return "OpFUnordLessThanEqual";
        case spv::OpFOrdGreaterThanEqual:
            return "OpFOrdGreaterThanEqual";
        case spv::OpFUnordGreaterThanEqual:
            return "OpFUnordGreaterThanEqual";
        case spv::OpShiftRightLogical:
            return "OpShiftRightLogical";
        case spv::OpShiftRightArithmetic:
            return "OpShiftRightArithmetic";
        case spv::OpShiftLeftLogical:
            return "OpShiftLeftLogical";
        case spv::OpBitwiseOr:
            return "OpBitwiseOr";
        case spv::OpBitwiseXor:
            return "OpBitwiseXor";
        case spv::OpBitwiseAnd:
            return "OpBitwiseAnd";
        case spv::OpNot:
            return "OpNot";
        case spv::OpBitFieldInsert:
            return "OpBitFieldInsert";
        case spv::OpBitFieldSExtract:
            return "OpBitFieldSExtract";
        case spv::OpBitFieldUExtract:
            return "OpBitFieldUExtract";
        case spv::OpBitReverse:
            return "OpBitReverse";
        case spv::OpBitCount:
            return "OpBitCount";
        case spv::OpDPdx:
            return "OpDPdx";
        case spv::OpDPdy:
            return "OpDPdy";
        case spv::OpFwidth:
            return "OpFwidth";
        case spv::OpDPdxFine:
            return "OpDPdxFine";
        case spv::OpDPdyFine:
            return "OpDPdyFine";
        case spv::OpFwidthFine:
            return "OpFwidthFine";
        case spv::OpDPdxCoarse:
            return "OpDPdxCoarse";
        case spv::OpDPdyCoarse:
            return "OpDPdyCoarse";
        case spv::OpFwidthCoarse:
            return "OpFwidthCoarse";
        case spv::OpEmitVertex:
            return "OpEmitVertex";
        case spv::OpEndPrimitive:
            return "OpEndPrimitive";
        case spv::OpEmitStreamVertex:
            return "OpEmitStreamVertex";
        case spv::OpEndStreamPrimitive:
            return "OpEndStreamPrimitive";
        case spv::OpControlBarrier:
            return "OpControlBarrier";
        case spv::OpMemoryBarrier:
            return "OpMemoryBarrier";
        case spv::OpAtomicLoad:
            return "OpAtomicLoad";
        case spv::OpAtomicStore:
            return "OpAtomicStore";
        case spv::OpAtomicExchange:
            return "OpAtomicExchange";
        case spv::OpAtomicCompareExchange:
            return "OpAtomicCompareExchange";
        case spv::OpAtomicIIncrement:
            return "OpAtomicIIncrement";
        case spv::OpAtomicIDecrement:
            return "OpAtomicIDecrement";
        case spv::OpAtomicIAdd:
            return "OpAtomicIAdd";
        case spv::OpAtomicISub:
            return "OpAtomicISub";
        case spv::OpAtomicSMin:
            return "OpAtomicSMin";
        case spv::OpAtomicUMin:
            return "OpAtomicUMin";
        case spv::OpAtomicSMax:
            return "OpAtomicSMax";
        case spv::OpAtomicUMax:
            return "OpAtomicUMax";
        case spv::OpAtomicAnd:
            return "OpAtomicAnd";
        case spv::OpAtomicOr:
            return "OpAtomicOr";
        case spv::OpAtomicXor:
            return "OpAtomicXor";
        case spv::OpPhi:
            return "OpPhi";
        case spv::OpLoopMerge:
            return "OpLoopMerge";
        case spv::OpSelectionMerge:
            return "OpSelectionMerge";
        case spv::OpLabel:
            return "OpLabel";
        case spv::OpBranch:
            return "OpBranch";
        case spv::OpBranchConditional:
            return "OpBranchConditional";
        case spv::OpSwitch:
            return "OpSwitch";
        case spv::OpKill:
            return "OpKill";
        case spv::OpReturn:
            return "OpReturn";
        case spv::OpReturnValue:
            return "OpReturnValue";
        case spv::OpUnreachable:
            return "OpUnreachable";
        case spv::OpGroupAll:
            return "OpGroupAll";
        case spv::OpGroupAny:
            return "OpGroupAny";
        case spv::OpGroupBroadcast:
            return "OpGroupBroadcast";
        case spv::OpGroupIAdd:
            return "OpGroupIAdd";
        case spv::OpGroupFAdd:
            return "OpGroupFAdd";
        case spv::OpGroupFMin:
            return "OpGroupFMin";
        case spv::OpGroupUMin:
            return "OpGroupUMin";
        case spv::OpGroupSMin:
            return "OpGroupSMin";
        case spv::OpGroupFMax:
            return "OpGroupFMax";
        case spv::OpGroupUMax:
            return "OpGroupUMax";
        case spv::OpGroupSMax:
            return "OpGroupSMax";
        case spv::OpImageSparseSampleImplicitLod:
            return "OpImageSparseSampleImplicitLod";
        case spv::OpImageSparseSampleExplicitLod:
            return "OpImageSparseSampleExplicitLod";
        case spv::OpImageSparseSampleDrefImplicitLod:
            return "OpImageSparseSampleDrefImplicitLod";
        case spv::OpImageSparseSampleDrefExplicitLod:
            return "OpImageSparseSampleDrefExplicitLod";
        case spv::OpImageSparseSampleProjImplicitLod:
            return "OpImageSparseSampleProjImplicitLod";
        case spv::OpImageSparseSampleProjExplicitLod:
            return "OpImageSparseSampleProjExplicitLod";
        case spv::OpImageSparseSampleProjDrefImplicitLod:
            return "OpImageSparseSampleProjDrefImplicitLod";
        case spv::OpImageSparseSampleProjDrefExplicitLod:
            return "OpImageSparseSampleProjDrefExplicitLod";
        case spv::OpImageSparseFetch:
            return "OpImageSparseFetch";
        case spv::OpImageSparseGather:
            return "OpImageSparseGather";
        case spv::OpImageSparseDrefGather:
            return "OpImageSparseDrefGather";
        case spv::OpImageSparseTexelsResident:
            return "OpImageSparseTexelsResident";
        case spv::OpNoLine:
            return "OpNoLine";
        case spv::OpImageSparseRead:
            return "OpImageSparseRead";
        case spv::OpSizeOf:
            return "OpSizeOf";
        case spv::OpModuleProcessed:
            return "OpModuleProcessed";
        case spv::OpExecutionModeId:
            return "OpExecutionModeId";
        case spv::OpDecorateId:
            return "OpDecorateId";
        case spv::OpGroupNonUniformElect:
            return "OpGroupNonUniformElect";
        case spv::OpGroupNonUniformAll:
            return "OpGroupNonUniformAll";
        case spv::OpGroupNonUniformAny:
            return "OpGroupNonUniformAny";
        case spv::OpGroupNonUniformAllEqual:
            return "OpGroupNonUniformAllEqual";
        case spv::OpGroupNonUniformBroadcast:
            return "OpGroupNonUniformBroadcast";
        case spv::OpGroupNonUniformBroadcastFirst:
            return "OpGroupNonUniformBroadcastFirst";
        case spv::OpGroupNonUniformBallot:
            return "OpGroupNonUniformBallot";
        case spv::OpGroupNonUniformInverseBallot:
            return "OpGroupNonUniformInverseBallot";
        case spv::OpGroupNonUniformBallotBitExtract:
            return "OpGroupNonUniformBallotBitExtract";
        case spv::OpGroupNonUniformBallotBitCount:
            return "OpGroupNonUniformBallotBitCount";
        case spv::OpGroupNonUniformBallotFindLSB:
            return "OpGroupNonUniformBallotFindLSB";
        case spv::OpGroupNonUniformBallotFindMSB:
            return "OpGroupNonUniformBallotFindMSB";
        case spv::OpGroupNonUniformShuffle:
            return "OpGroupNonUniformShuffle";
        case spv::OpGroupNonUniformShuffleXor:
            return "OpGroupNonUniformShuffleXor";
        case spv::OpGroupNonUniformShuffleUp:
            return "OpGroupNonUniformShuffleUp";
        case spv::OpGroupNonUniformShuffleDown:
            return "OpGroupNonUniformShuffleDown";
        case spv::OpGroupNonUniformIAdd:
            return "OpGroupNonUniformIAdd";
        case spv::OpGroupNonUniformFAdd:
            return "OpGroupNonUniformFAdd";
        case spv::OpGroupNonUniformIMul:
            return "OpGroupNonUniformIMul";
        case spv::OpGroupNonUniformFMul:
            return "OpGroupNonUniformFMul";
        case spv::OpGroupNonUniformSMin:
            return "OpGroupNonUniformSMin";
        case spv::OpGroupNonUniformUMin:
            return "OpGroupNonUniformUMin";
        case spv::OpGroupNonUniformFMin:
            return "OpGroupNonUniformFMin";
        case spv::OpGroupNonUniformSMax:
            return "OpGroupNonUniformSMax";
        case spv::OpGroupNonUniformUMax:
            return "OpGroupNonUniformUMax";
        case spv::OpGroupNonUniformFMax:
            return "OpGroupNonUniformFMax";
        case spv::OpGroupNonUniformBitwiseAnd:
            return "OpGroupNonUniformBitwiseAnd";
        case spv::OpGroupNonUniformBitwiseOr:
            return "OpGroupNonUniformBitwiseOr";
        case spv::OpGroupNonUniformBitwiseXor:
            return "OpGroupNonUniformBitwiseXor";
        case spv::OpGroupNonUniformLogicalAnd:
            return "OpGroupNonUniformLogicalAnd";
        case spv::OpGroupNonUniformLogicalOr:
            return "OpGroupNonUniformLogicalOr";
        case spv::OpGroupNonUniformLogicalXor:
            return "OpGroupNonUniformLogicalXor";
        case spv::OpGroupNonUniformQuadBroadcast:
            return "OpGroupNonUniformQuadBroadcast";
        case spv::OpGroupNonUniformQuadSwap:
            return "OpGroupNonUniformQuadSwap";
        case spv::OpCopyLogical:
            return "OpCopyLogical";
        case spv::OpPtrEqual:
            return "OpPtrEqual";
        case spv::OpPtrNotEqual:
            return "OpPtrNotEqual";
        case spv::OpPtrDiff:
            return "OpPtrDiff";
        case spv::OpColorAttachmentReadEXT:
            return "OpColorAttachmentReadEXT";
        case spv::OpDepthAttachmentReadEXT:
            return "OpDepthAttachmentReadEXT";
        case spv::OpStencilAttachmentReadEXT:
            return "OpStencilAttachmentReadEXT";
        case spv::OpTerminateInvocation:
            return "OpTerminateInvocation";
        case spv::OpSubgroupBallotKHR:
            return "OpSubgroupBallotKHR";
        case spv::OpSubgroupFirstInvocationKHR:
            return "OpSubgroupFirstInvocationKHR";
        case spv::OpSubgroupAllKHR:
            return "OpSubgroupAllKHR";
        case spv::OpSubgroupAnyKHR:
            return "OpSubgroupAnyKHR";
        case spv::OpSubgroupAllEqualKHR:
            return "OpSubgroupAllEqualKHR";
        case spv::OpGroupNonUniformRotateKHR:
            return "OpGroupNonUniformRotateKHR";
        case spv::OpSubgroupReadInvocationKHR:
            return "OpSubgroupReadInvocationKHR";
        case spv::OpExtInstWithForwardRefsKHR:
            return "OpExtInstWithForwardRefsKHR";
        case spv::OpTraceRayKHR:
            return "OpTraceRayKHR";
        case spv::OpExecuteCallableKHR:
            return "OpExecuteCallableKHR";
        case spv::OpConvertUToAccelerationStructureKHR:
            return "OpConvertUToAccelerationStructureKHR";
        case spv::OpIgnoreIntersectionKHR:
            return "OpIgnoreIntersectionKHR";
        case spv::OpTerminateRayKHR:
            return "OpTerminateRayKHR";
        case spv::OpSDot:
            return "OpSDot";
        case spv::OpUDot:
            return "OpUDot";
        case spv::OpSUDot:
            return "OpSUDot";
        case spv::OpSDotAccSat:
            return "OpSDotAccSat";
        case spv::OpUDotAccSat:
            return "OpUDotAccSat";
        case spv::OpSUDotAccSat:
            return "OpSUDotAccSat";
        case spv::OpTypeCooperativeMatrixKHR:
            return "OpTypeCooperativeMatrixKHR";
        case spv::OpCooperativeMatrixLoadKHR:
            return "OpCooperativeMatrixLoadKHR";
        case spv::OpCooperativeMatrixStoreKHR:
            return "OpCooperativeMatrixStoreKHR";
        case spv::OpCooperativeMatrixMulAddKHR:
            return "OpCooperativeMatrixMulAddKHR";
        case spv::OpCooperativeMatrixLengthKHR:
            return "OpCooperativeMatrixLengthKHR";
        case spv::OpConstantCompositeReplicateEXT:
            return "OpConstantCompositeReplicateEXT";
        case spv::OpSpecConstantCompositeReplicateEXT:
            return "OpSpecConstantCompositeReplicateEXT";
        case spv::OpCompositeConstructReplicateEXT:
            return "OpCompositeConstructReplicateEXT";
        case spv::OpTypeRayQueryKHR:
            return "OpTypeRayQueryKHR";
        case spv::OpRayQueryInitializeKHR:
            return "OpRayQueryInitializeKHR";
        case spv::OpRayQueryTerminateKHR:
            return "OpRayQueryTerminateKHR";
        case spv::OpRayQueryGenerateIntersectionKHR:
            return "OpRayQueryGenerateIntersectionKHR";
        case spv::OpRayQueryConfirmIntersectionKHR:
            return "OpRayQueryConfirmIntersectionKHR";
        case spv::OpRayQueryProceedKHR:
            return "OpRayQueryProceedKHR";
        case spv::OpRayQueryGetIntersectionTypeKHR:
            return "OpRayQueryGetIntersectionTypeKHR";
        case spv::OpImageSampleWeightedQCOM:
            return "OpImageSampleWeightedQCOM";
        case spv::OpImageBoxFilterQCOM:
            return "OpImageBoxFilterQCOM";
        case spv::OpImageBlockMatchSSDQCOM:
            return "OpImageBlockMatchSSDQCOM";
        case spv::OpImageBlockMatchSADQCOM:
            return "OpImageBlockMatchSADQCOM";
        case spv::OpImageBlockMatchWindowSSDQCOM:
            return "OpImageBlockMatchWindowSSDQCOM";
        case spv::OpImageBlockMatchWindowSADQCOM:
            return "OpImageBlockMatchWindowSADQCOM";
        case spv::OpImageBlockMatchGatherSSDQCOM:
            return "OpImageBlockMatchGatherSSDQCOM";
        case spv::OpImageBlockMatchGatherSADQCOM:
            return "OpImageBlockMatchGatherSADQCOM";
        case spv::OpGroupIAddNonUniformAMD:
            return "OpGroupIAddNonUniformAMD";
        case spv::OpGroupFAddNonUniformAMD:
            return "OpGroupFAddNonUniformAMD";
        case spv::OpGroupFMinNonUniformAMD:
            return "OpGroupFMinNonUniformAMD";
        case spv::OpGroupUMinNonUniformAMD:
            return "OpGroupUMinNonUniformAMD";
        case spv::OpGroupSMinNonUniformAMD:
            return "OpGroupSMinNonUniformAMD";
        case spv::OpGroupFMaxNonUniformAMD:
            return "OpGroupFMaxNonUniformAMD";
        case spv::OpGroupUMaxNonUniformAMD:
            return "OpGroupUMaxNonUniformAMD";
        case spv::OpGroupSMaxNonUniformAMD:
            return "OpGroupSMaxNonUniformAMD";
        case spv::OpFragmentMaskFetchAMD:
            return "OpFragmentMaskFetchAMD";
        case spv::OpFragmentFetchAMD:
            return "OpFragmentFetchAMD";
        case spv::OpReadClockKHR:
            return "OpReadClockKHR";
        case spv::OpGroupNonUniformQuadAllKHR:
            return "OpGroupNonUniformQuadAllKHR";
        case spv::OpGroupNonUniformQuadAnyKHR:
            return "OpGroupNonUniformQuadAnyKHR";
        case spv::OpHitObjectRecordHitMotionNV:
            return "OpHitObjectRecordHitMotionNV";
        case spv::OpHitObjectRecordHitWithIndexMotionNV:
            return "OpHitObjectRecordHitWithIndexMotionNV";
        case spv::OpHitObjectRecordMissMotionNV:
            return "OpHitObjectRecordMissMotionNV";
        case spv::OpHitObjectGetWorldToObjectNV:
            return "OpHitObjectGetWorldToObjectNV";
        case spv::OpHitObjectGetObjectToWorldNV:
            return "OpHitObjectGetObjectToWorldNV";
        case spv::OpHitObjectGetObjectRayDirectionNV:
            return "OpHitObjectGetObjectRayDirectionNV";
        case spv::OpHitObjectGetObjectRayOriginNV:
            return "OpHitObjectGetObjectRayOriginNV";
        case spv::OpHitObjectTraceRayMotionNV:
            return "OpHitObjectTraceRayMotionNV";
        case spv::OpHitObjectGetShaderRecordBufferHandleNV:
            return "OpHitObjectGetShaderRecordBufferHandleNV";
        case spv::OpHitObjectGetShaderBindingTableRecordIndexNV:
            return "OpHitObjectGetShaderBindingTableRecordIndexNV";
        case spv::OpHitObjectRecordEmptyNV:
            return "OpHitObjectRecordEmptyNV";
        case spv::OpHitObjectTraceRayNV:
            return "OpHitObjectTraceRayNV";
        case spv::OpHitObjectRecordHitNV:
            return "OpHitObjectRecordHitNV";
        case spv::OpHitObjectRecordHitWithIndexNV:
            return "OpHitObjectRecordHitWithIndexNV";
        case spv::OpHitObjectRecordMissNV:
            return "OpHitObjectRecordMissNV";
        case spv::OpHitObjectExecuteShaderNV:
            return "OpHitObjectExecuteShaderNV";
        case spv::OpHitObjectGetCurrentTimeNV:
            return "OpHitObjectGetCurrentTimeNV";
        case spv::OpHitObjectGetAttributesNV:
            return "OpHitObjectGetAttributesNV";
        case spv::OpHitObjectGetHitKindNV:
            return "OpHitObjectGetHitKindNV";
        case spv::OpHitObjectGetPrimitiveIndexNV:
            return "OpHitObjectGetPrimitiveIndexNV";
        case spv::OpHitObjectGetGeometryIndexNV:
            return "OpHitObjectGetGeometryIndexNV";
        case spv::OpHitObjectGetInstanceIdNV:
            return "OpHitObjectGetInstanceIdNV";
        case spv::OpHitObjectGetInstanceCustomIndexNV:
            return "OpHitObjectGetInstanceCustomIndexNV";
        case spv::OpHitObjectGetWorldRayDirectionNV:
            return "OpHitObjectGetWorldRayDirectionNV";
        case spv::OpHitObjectGetWorldRayOriginNV:
            return "OpHitObjectGetWorldRayOriginNV";
        case spv::OpHitObjectGetRayTMaxNV:
            return "OpHitObjectGetRayTMaxNV";
        case spv::OpHitObjectGetRayTMinNV:
            return "OpHitObjectGetRayTMinNV";
        case spv::OpHitObjectIsEmptyNV:
            return "OpHitObjectIsEmptyNV";
        case spv::OpHitObjectIsHitNV:
            return "OpHitObjectIsHitNV";
        case spv::OpHitObjectIsMissNV:
            return "OpHitObjectIsMissNV";
        case spv::OpReorderThreadWithHitObjectNV:
            return "OpReorderThreadWithHitObjectNV";
        case spv::OpReorderThreadWithHintNV:
            return "OpReorderThreadWithHintNV";
        case spv::OpTypeHitObjectNV:
            return "OpTypeHitObjectNV";
        case spv::OpImageSampleFootprintNV:
            return "OpImageSampleFootprintNV";
        case spv::OpTypeCooperativeVectorNV:
            return "OpTypeCooperativeVectorNV";
        case spv::OpCooperativeVectorMatrixMulNV:
            return "OpCooperativeVectorMatrixMulNV";
        case spv::OpCooperativeVectorOuterProductAccumulateNV:
            return "OpCooperativeVectorOuterProductAccumulateNV";
        case spv::OpCooperativeVectorReduceSumAccumulateNV:
            return "OpCooperativeVectorReduceSumAccumulateNV";
        case spv::OpCooperativeVectorMatrixMulAddNV:
            return "OpCooperativeVectorMatrixMulAddNV";
        case spv::OpCooperativeMatrixConvertNV:
            return "OpCooperativeMatrixConvertNV";
        case spv::OpEmitMeshTasksEXT:
            return "OpEmitMeshTasksEXT";
        case spv::OpSetMeshOutputsEXT:
            return "OpSetMeshOutputsEXT";
        case spv::OpGroupNonUniformPartitionNV:
            return "OpGroupNonUniformPartitionNV";
        case spv::OpWritePackedPrimitiveIndices4x8NV:
            return "OpWritePackedPrimitiveIndices4x8NV";
        case spv::OpFetchMicroTriangleVertexPositionNV:
            return "OpFetchMicroTriangleVertexPositionNV";
        case spv::OpFetchMicroTriangleVertexBarycentricNV:
            return "OpFetchMicroTriangleVertexBarycentricNV";
        case spv::OpCooperativeVectorLoadNV:
            return "OpCooperativeVectorLoadNV";
        case spv::OpCooperativeVectorStoreNV:
            return "OpCooperativeVectorStoreNV";
        case spv::OpReportIntersectionKHR:
            return "OpReportIntersectionKHR";
        case spv::OpIgnoreIntersectionNV:
            return "OpIgnoreIntersectionNV";
        case spv::OpTerminateRayNV:
            return "OpTerminateRayNV";
        case spv::OpTraceNV:
            return "OpTraceNV";
        case spv::OpTraceMotionNV:
            return "OpTraceMotionNV";
        case spv::OpTraceRayMotionNV:
            return "OpTraceRayMotionNV";
        case spv::OpRayQueryGetIntersectionTriangleVertexPositionsKHR:
            return "OpRayQueryGetIntersectionTriangleVertexPositionsKHR";
        case spv::OpTypeAccelerationStructureKHR:
            return "OpTypeAccelerationStructureKHR";
        case spv::OpExecuteCallableNV:
            return "OpExecuteCallableNV";
        case spv::OpRayQueryGetClusterIdNV:
            return "OpRayQueryGetClusterIdNV";
        case spv::OpHitObjectGetClusterIdNV:
            return "OpHitObjectGetClusterIdNV";
        case spv::OpTypeCooperativeMatrixNV:
            return "OpTypeCooperativeMatrixNV";
        case spv::OpCooperativeMatrixLoadNV:
            return "OpCooperativeMatrixLoadNV";
        case spv::OpCooperativeMatrixStoreNV:
            return "OpCooperativeMatrixStoreNV";
        case spv::OpCooperativeMatrixMulAddNV:
            return "OpCooperativeMatrixMulAddNV";
        case spv::OpCooperativeMatrixLengthNV:
            return "OpCooperativeMatrixLengthNV";
        case spv::OpBeginInvocationInterlockEXT:
            return "OpBeginInvocationInterlockEXT";
        case spv::OpEndInvocationInterlockEXT:
            return "OpEndInvocationInterlockEXT";
        case spv::OpCooperativeMatrixReduceNV:
            return "OpCooperativeMatrixReduceNV";
        case spv::OpCooperativeMatrixLoadTensorNV:
            return "OpCooperativeMatrixLoadTensorNV";
        case spv::OpCooperativeMatrixStoreTensorNV:
            return "OpCooperativeMatrixStoreTensorNV";
        case spv::OpCooperativeMatrixPerElementOpNV:
            return "OpCooperativeMatrixPerElementOpNV";
        case spv::OpTypeTensorLayoutNV:
            return "OpTypeTensorLayoutNV";
        case spv::OpTypeTensorViewNV:
            return "OpTypeTensorViewNV";
        case spv::OpCreateTensorLayoutNV:
            return "OpCreateTensorLayoutNV";
        case spv::OpTensorLayoutSetDimensionNV:
            return "OpTensorLayoutSetDimensionNV";
        case spv::OpTensorLayoutSetStrideNV:
            return "OpTensorLayoutSetStrideNV";
        case spv::OpTensorLayoutSliceNV:
            return "OpTensorLayoutSliceNV";
        case spv::OpTensorLayoutSetClampValueNV:
            return "OpTensorLayoutSetClampValueNV";
        case spv::OpCreateTensorViewNV:
            return "OpCreateTensorViewNV";
        case spv::OpTensorViewSetDimensionNV:
            return "OpTensorViewSetDimensionNV";
        case spv::OpTensorViewSetStrideNV:
            return "OpTensorViewSetStrideNV";
        case spv::OpDemoteToHelperInvocation:
            return "OpDemoteToHelperInvocation";
        case spv::OpIsHelperInvocationEXT:
            return "OpIsHelperInvocationEXT";
        case spv::OpTensorViewSetClipNV:
            return "OpTensorViewSetClipNV";
        case spv::OpTensorLayoutSetBlockSizeNV:
            return "OpTensorLayoutSetBlockSizeNV";
        case spv::OpCooperativeMatrixTransposeNV:
            return "OpCooperativeMatrixTransposeNV";
        case spv::OpConvertUToImageNV:
            return "OpConvertUToImageNV";
        case spv::OpConvertUToSamplerNV:
            return "OpConvertUToSamplerNV";
        case spv::OpConvertImageToUNV:
            return "OpConvertImageToUNV";
        case spv::OpConvertSamplerToUNV:
            return "OpConvertSamplerToUNV";
        case spv::OpConvertUToSampledImageNV:
            return "OpConvertUToSampledImageNV";
        case spv::OpConvertSampledImageToUNV:
            return "OpConvertSampledImageToUNV";
        case spv::OpSamplerImageAddressingModeNV:
            return "OpSamplerImageAddressingModeNV";
        case spv::OpRawAccessChainNV:
            return "OpRawAccessChainNV";
        case spv::OpRayQueryGetIntersectionSpherePositionNV:
            return "OpRayQueryGetIntersectionSpherePositionNV";
        case spv::OpRayQueryGetIntersectionSphereRadiusNV:
            return "OpRayQueryGetIntersectionSphereRadiusNV";
        case spv::OpRayQueryGetIntersectionLSSPositionsNV:
            return "OpRayQueryGetIntersectionLSSPositionsNV";
        case spv::OpRayQueryGetIntersectionLSSRadiiNV:
            return "OpRayQueryGetIntersectionLSSRadiiNV";
        case spv::OpRayQueryGetIntersectionLSSHitValueNV:
            return "OpRayQueryGetIntersectionLSSHitValueNV";
        case spv::OpHitObjectGetSpherePositionNV:
            return "OpHitObjectGetSpherePositionNV";
        case spv::OpHitObjectGetSphereRadiusNV:
            return "OpHitObjectGetSphereRadiusNV";
        case spv::OpHitObjectGetLSSPositionsNV:
            return "OpHitObjectGetLSSPositionsNV";
        case spv::OpHitObjectGetLSSRadiiNV:
            return "OpHitObjectGetLSSRadiiNV";
        case spv::OpHitObjectIsSphereHitNV:
            return "OpHitObjectIsSphereHitNV";
        case spv::OpHitObjectIsLSSHitNV:
            return "OpHitObjectIsLSSHitNV";
        case spv::OpRayQueryIsSphereHitNV:
            return "OpRayQueryIsSphereHitNV";
        case spv::OpRayQueryIsLSSHitNV:
            return "OpRayQueryIsLSSHitNV";
        case spv::OpUCountLeadingZerosINTEL:
            return "OpUCountLeadingZerosINTEL";
        case spv::OpUCountTrailingZerosINTEL:
            return "OpUCountTrailingZerosINTEL";
        case spv::OpAbsISubINTEL:
            return "OpAbsISubINTEL";
        case spv::OpAbsUSubINTEL:
            return "OpAbsUSubINTEL";
        case spv::OpIAddSatINTEL:
            return "OpIAddSatINTEL";
        case spv::OpUAddSatINTEL:
            return "OpUAddSatINTEL";
        case spv::OpIAverageINTEL:
            return "OpIAverageINTEL";
        case spv::OpUAverageINTEL:
            return "OpUAverageINTEL";
        case spv::OpIAverageRoundedINTEL:
            return "OpIAverageRoundedINTEL";
        case spv::OpUAverageRoundedINTEL:
            return "OpUAverageRoundedINTEL";
        case spv::OpISubSatINTEL:
            return "OpISubSatINTEL";
        case spv::OpUSubSatINTEL:
            return "OpUSubSatINTEL";
        case spv::OpIMul32x16INTEL:
            return "OpIMul32x16INTEL";
        case spv::OpUMul32x16INTEL:
            return "OpUMul32x16INTEL";
        case spv::OpConstantFunctionPointerINTEL:
            return "OpConstantFunctionPointerINTEL";
        case spv::OpFunctionPointerCallINTEL:
            return "OpFunctionPointerCallINTEL";
        case spv::OpAtomicFMinEXT:
            return "OpAtomicFMinEXT";
        case spv::OpAtomicFMaxEXT:
            return "OpAtomicFMaxEXT";
        case spv::OpAssumeTrueKHR:
            return "OpAssumeTrueKHR";
        case spv::OpExpectKHR:
            return "OpExpectKHR";
        case spv::OpDecorateString:
            return "OpDecorateString";
        case spv::OpMemberDecorateString:
            return "OpMemberDecorateString";
        case spv::OpRayQueryGetRayTMinKHR:
            return "OpRayQueryGetRayTMinKHR";
        case spv::OpRayQueryGetRayFlagsKHR:
            return "OpRayQueryGetRayFlagsKHR";
        case spv::OpRayQueryGetIntersectionTKHR:
            return "OpRayQueryGetIntersectionTKHR";
        case spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR:
            return "OpRayQueryGetIntersectionInstanceCustomIndexKHR";
        case spv::OpRayQueryGetIntersectionInstanceIdKHR:
            return "OpRayQueryGetIntersectionInstanceIdKHR";
        case spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR:
            return "OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR";
        case spv::OpRayQueryGetIntersectionGeometryIndexKHR:
            return "OpRayQueryGetIntersectionGeometryIndexKHR";
        case spv::OpRayQueryGetIntersectionPrimitiveIndexKHR:
            return "OpRayQueryGetIntersectionPrimitiveIndexKHR";
        case spv::OpRayQueryGetIntersectionBarycentricsKHR:
            return "OpRayQueryGetIntersectionBarycentricsKHR";
        case spv::OpRayQueryGetIntersectionFrontFaceKHR:
            return "OpRayQueryGetIntersectionFrontFaceKHR";
        case spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR:
            return "OpRayQueryGetIntersectionCandidateAABBOpaqueKHR";
        case spv::OpRayQueryGetIntersectionObjectRayDirectionKHR:
            return "OpRayQueryGetIntersectionObjectRayDirectionKHR";
        case spv::OpRayQueryGetIntersectionObjectRayOriginKHR:
            return "OpRayQueryGetIntersectionObjectRayOriginKHR";
        case spv::OpRayQueryGetWorldRayDirectionKHR:
            return "OpRayQueryGetWorldRayDirectionKHR";
        case spv::OpRayQueryGetWorldRayOriginKHR:
            return "OpRayQueryGetWorldRayOriginKHR";
        case spv::OpRayQueryGetIntersectionObjectToWorldKHR:
            return "OpRayQueryGetIntersectionObjectToWorldKHR";
        case spv::OpRayQueryGetIntersectionWorldToObjectKHR:
            return "OpRayQueryGetIntersectionWorldToObjectKHR";
        case spv::OpAtomicFAddEXT:
            return "OpAtomicFAddEXT";
        case spv::OpArithmeticFenceEXT:
            return "OpArithmeticFenceEXT";
        case spv::OpSubgroupBlockPrefetchINTEL:
            return "OpSubgroupBlockPrefetchINTEL";
        case spv::OpSubgroup2DBlockLoadINTEL:
            return "OpSubgroup2DBlockLoadINTEL";
        case spv::OpSubgroup2DBlockLoadTransformINTEL:
            return "OpSubgroup2DBlockLoadTransformINTEL";
        case spv::OpSubgroup2DBlockLoadTransposeINTEL:
            return "OpSubgroup2DBlockLoadTransposeINTEL";
        case spv::OpSubgroup2DBlockPrefetchINTEL:
            return "OpSubgroup2DBlockPrefetchINTEL";
        case spv::OpSubgroup2DBlockStoreINTEL:
            return "OpSubgroup2DBlockStoreINTEL";
        case spv::OpSubgroupMatrixMultiplyAccumulateINTEL:
            return "OpSubgroupMatrixMultiplyAccumulateINTEL";
        case spv::OpGroupIMulKHR:
            return "OpGroupIMulKHR";
        case spv::OpGroupFMulKHR:
            return "OpGroupFMulKHR";
        case spv::OpGroupBitwiseAndKHR:
            return "OpGroupBitwiseAndKHR";
        case spv::OpGroupBitwiseOrKHR:
            return "OpGroupBitwiseOrKHR";
        case spv::OpGroupBitwiseXorKHR:
            return "OpGroupBitwiseXorKHR";
        case spv::OpGroupLogicalAndKHR:
            return "OpGroupLogicalAndKHR";
        case spv::OpGroupLogicalOrKHR:
            return "OpGroupLogicalOrKHR";
        case spv::OpGroupLogicalXorKHR:
            return "OpGroupLogicalXorKHR";

#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::OpTypeUntypedPointerKHR:
            return "OpTypeUntypedPointerKHR";
        case spv::OpUntypedVariableKHR:
            return "OpUntypedVariableKHR";
        case spv::OpUntypedAccessChainKHR:
            return "OpUntypedAccessChainKHR";
        case spv::OpUntypedInBoundsAccessChainKHR:
            return "OpUntypedInBoundsAccessChainKHR";
        case spv::OpUntypedPtrAccessChainKHR:
            return "OpUntypedPtrAccessChainKHR";
        case spv::OpUntypedInBoundsPtrAccessChainKHR:
            return "OpUntypedInBoundsPtrAccessChainKHR";
        case spv::OpUntypedArrayLengthKHR:
            return "OpUntypedArrayLengthKHR";
        case spv::OpUntypedPrefetchKHR:
            return "OpUntypedPrefetchKHR";
        case spv::OpAllocateNodePayloadsAMDX:
            return "OpAllocateNodePayloadsAMDX";
        case spv::OpEnqueueNodePayloadsAMDX:
            return "OpEnqueueNodePayloadsAMDX";
        case spv::OpTypeNodePayloadArrayAMDX:
            return "OpTypeNodePayloadArrayAMDX";
        case spv::OpFinishWritingNodePayloadAMDX:
            return "OpFinishWritingNodePayloadAMDX";
        case spv::OpNodePayloadArrayLengthAMDX:
            return "OpNodePayloadArrayLengthAMDX";
        case spv::OpIsNodePayloadValidAMDX:
            return "OpIsNodePayloadValidAMDX";
        case spv::OpConstantStringAMDX:
            return "OpConstantStringAMDX";
        case spv::OpSpecConstantStringAMDX:
            return "OpSpecConstantStringAMDX";
#endif
        default:
            return "Unknown Opcode";
    }
}

const char* string_SpvStorageClass(uint32_t storage_class) {
    switch (storage_class) {
        case spv::StorageClassUniformConstant:
            return "UniformConstant";
        case spv::StorageClassInput:
            return "Input";
        case spv::StorageClassUniform:
            return "Uniform";
        case spv::StorageClassOutput:
            return "Output";
        case spv::StorageClassWorkgroup:
            return "Workgroup";
        case spv::StorageClassCrossWorkgroup:
            return "CrossWorkgroup";
        case spv::StorageClassPrivate:
            return "Private";
        case spv::StorageClassFunction:
            return "Function";
        case spv::StorageClassGeneric:
            return "Generic";
        case spv::StorageClassPushConstant:
            return "PushConstant";
        case spv::StorageClassAtomicCounter:
            return "AtomicCounter";
        case spv::StorageClassImage:
            return "Image";
        case spv::StorageClassStorageBuffer:
            return "StorageBuffer";
        case spv::StorageClassTileImageEXT:
            return "TileImageEXT";
        case spv::StorageClassCallableDataKHR:
            return "CallableDataKHR";
        case spv::StorageClassIncomingCallableDataKHR:
            return "IncomingCallableDataKHR";
        case spv::StorageClassRayPayloadKHR:
            return "RayPayloadKHR";
        case spv::StorageClassHitAttributeKHR:
            return "HitAttributeKHR";
        case spv::StorageClassIncomingRayPayloadKHR:
            return "IncomingRayPayloadKHR";
        case spv::StorageClassShaderRecordBufferKHR:
            return "ShaderRecordBufferKHR";
        case spv::StorageClassPhysicalStorageBuffer:
            return "PhysicalStorageBuffer";
        case spv::StorageClassHitObjectAttributeNV:
            return "HitObjectAttributeNV";
        case spv::StorageClassTaskPayloadWorkgroupEXT:
            return "TaskPayloadWorkgroupEXT";
        case spv::StorageClassCodeSectionINTEL:
            return "CodeSectionINTEL";
        case spv::StorageClassDeviceOnlyINTEL:
            return "DeviceOnlyINTEL";
        case spv::StorageClassHostOnlyINTEL:
            return "HostOnlyINTEL";

#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::StorageClassNodePayloadAMDX:
            return "NodePayloadAMDX";
#endif
        default:
            return "Unknown Storage Class";
    }
}

const char* string_SpvExecutionModel(uint32_t execution_model) {
    switch (execution_model) {
        case spv::ExecutionModelVertex:
            return "Vertex";
        case spv::ExecutionModelTessellationControl:
            return "TessellationControl";
        case spv::ExecutionModelTessellationEvaluation:
            return "TessellationEvaluation";
        case spv::ExecutionModelGeometry:
            return "Geometry";
        case spv::ExecutionModelFragment:
            return "Fragment";
        case spv::ExecutionModelGLCompute:
            return "GLCompute";
        case spv::ExecutionModelKernel:
            return "Kernel";
        case spv::ExecutionModelTaskNV:
            return "TaskNV";
        case spv::ExecutionModelMeshNV:
            return "MeshNV";
        case spv::ExecutionModelRayGenerationKHR:
            return "RayGenerationKHR";
        case spv::ExecutionModelIntersectionKHR:
            return "IntersectionKHR";
        case spv::ExecutionModelAnyHitKHR:
            return "AnyHitKHR";
        case spv::ExecutionModelClosestHitKHR:
            return "ClosestHitKHR";
        case spv::ExecutionModelMissKHR:
            return "MissKHR";
        case spv::ExecutionModelCallableKHR:
            return "CallableKHR";
        case spv::ExecutionModelTaskEXT:
            return "TaskEXT";
        case spv::ExecutionModelMeshEXT:
            return "MeshEXT";

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif
        default:
            return "Unknown Execution Model";
    }
}

const char* string_SpvExecutionMode(uint32_t execution_mode) {
    switch (execution_mode) {
        case spv::ExecutionModeInvocations:
            return "Invocations";
        case spv::ExecutionModeSpacingEqual:
            return "SpacingEqual";
        case spv::ExecutionModeSpacingFractionalEven:
            return "SpacingFractionalEven";
        case spv::ExecutionModeSpacingFractionalOdd:
            return "SpacingFractionalOdd";
        case spv::ExecutionModeVertexOrderCw:
            return "VertexOrderCw";
        case spv::ExecutionModeVertexOrderCcw:
            return "VertexOrderCcw";
        case spv::ExecutionModePixelCenterInteger:
            return "PixelCenterInteger";
        case spv::ExecutionModeOriginUpperLeft:
            return "OriginUpperLeft";
        case spv::ExecutionModeOriginLowerLeft:
            return "OriginLowerLeft";
        case spv::ExecutionModeEarlyFragmentTests:
            return "EarlyFragmentTests";
        case spv::ExecutionModePointMode:
            return "PointMode";
        case spv::ExecutionModeXfb:
            return "Xfb";
        case spv::ExecutionModeDepthReplacing:
            return "DepthReplacing";
        case spv::ExecutionModeDepthGreater:
            return "DepthGreater";
        case spv::ExecutionModeDepthLess:
            return "DepthLess";
        case spv::ExecutionModeDepthUnchanged:
            return "DepthUnchanged";
        case spv::ExecutionModeLocalSize:
            return "LocalSize";
        case spv::ExecutionModeLocalSizeHint:
            return "LocalSizeHint";
        case spv::ExecutionModeInputPoints:
            return "InputPoints";
        case spv::ExecutionModeInputLines:
            return "InputLines";
        case spv::ExecutionModeInputLinesAdjacency:
            return "InputLinesAdjacency";
        case spv::ExecutionModeTriangles:
            return "Triangles";
        case spv::ExecutionModeInputTrianglesAdjacency:
            return "InputTrianglesAdjacency";
        case spv::ExecutionModeQuads:
            return "Quads";
        case spv::ExecutionModeIsolines:
            return "Isolines";
        case spv::ExecutionModeOutputVertices:
            return "OutputVertices";
        case spv::ExecutionModeOutputPoints:
            return "OutputPoints";
        case spv::ExecutionModeOutputLineStrip:
            return "OutputLineStrip";
        case spv::ExecutionModeOutputTriangleStrip:
            return "OutputTriangleStrip";
        case spv::ExecutionModeVecTypeHint:
            return "VecTypeHint";
        case spv::ExecutionModeContractionOff:
            return "ContractionOff";
        case spv::ExecutionModeInitializer:
            return "Initializer";
        case spv::ExecutionModeFinalizer:
            return "Finalizer";
        case spv::ExecutionModeSubgroupSize:
            return "SubgroupSize";
        case spv::ExecutionModeSubgroupsPerWorkgroup:
            return "SubgroupsPerWorkgroup";
        case spv::ExecutionModeSubgroupsPerWorkgroupId:
            return "SubgroupsPerWorkgroupId";
        case spv::ExecutionModeLocalSizeId:
            return "LocalSizeId";
        case spv::ExecutionModeLocalSizeHintId:
            return "LocalSizeHintId";
        case spv::ExecutionModeNonCoherentColorAttachmentReadEXT:
            return "NonCoherentColorAttachmentReadEXT";
        case spv::ExecutionModeNonCoherentDepthAttachmentReadEXT:
            return "NonCoherentDepthAttachmentReadEXT";
        case spv::ExecutionModeNonCoherentStencilAttachmentReadEXT:
            return "NonCoherentStencilAttachmentReadEXT";
        case spv::ExecutionModeSubgroupUniformControlFlowKHR:
            return "SubgroupUniformControlFlowKHR";
        case spv::ExecutionModePostDepthCoverage:
            return "PostDepthCoverage";
        case spv::ExecutionModeDenormPreserve:
            return "DenormPreserve";
        case spv::ExecutionModeDenormFlushToZero:
            return "DenormFlushToZero";
        case spv::ExecutionModeSignedZeroInfNanPreserve:
            return "SignedZeroInfNanPreserve";
        case spv::ExecutionModeRoundingModeRTE:
            return "RoundingModeRTE";
        case spv::ExecutionModeRoundingModeRTZ:
            return "RoundingModeRTZ";
        case spv::ExecutionModeEarlyAndLateFragmentTestsAMD:
            return "EarlyAndLateFragmentTestsAMD";
        case spv::ExecutionModeStencilRefReplacingEXT:
            return "StencilRefReplacingEXT";
        case spv::ExecutionModeStencilRefUnchangedFrontAMD:
            return "StencilRefUnchangedFrontAMD";
        case spv::ExecutionModeStencilRefGreaterFrontAMD:
            return "StencilRefGreaterFrontAMD";
        case spv::ExecutionModeStencilRefLessFrontAMD:
            return "StencilRefLessFrontAMD";
        case spv::ExecutionModeStencilRefUnchangedBackAMD:
            return "StencilRefUnchangedBackAMD";
        case spv::ExecutionModeStencilRefGreaterBackAMD:
            return "StencilRefGreaterBackAMD";
        case spv::ExecutionModeStencilRefLessBackAMD:
            return "StencilRefLessBackAMD";
        case spv::ExecutionModeQuadDerivativesKHR:
            return "QuadDerivativesKHR";
        case spv::ExecutionModeRequireFullQuadsKHR:
            return "RequireFullQuadsKHR";
        case spv::ExecutionModeOutputLinesEXT:
            return "OutputLinesEXT";
        case spv::ExecutionModeOutputPrimitivesEXT:
            return "OutputPrimitivesEXT";
        case spv::ExecutionModeDerivativeGroupQuadsKHR:
            return "DerivativeGroupQuadsKHR";
        case spv::ExecutionModeDerivativeGroupLinearKHR:
            return "DerivativeGroupLinearKHR";
        case spv::ExecutionModeOutputTrianglesEXT:
            return "OutputTrianglesEXT";
        case spv::ExecutionModePixelInterlockOrderedEXT:
            return "PixelInterlockOrderedEXT";
        case spv::ExecutionModePixelInterlockUnorderedEXT:
            return "PixelInterlockUnorderedEXT";
        case spv::ExecutionModeSampleInterlockOrderedEXT:
            return "SampleInterlockOrderedEXT";
        case spv::ExecutionModeSampleInterlockUnorderedEXT:
            return "SampleInterlockUnorderedEXT";
        case spv::ExecutionModeShadingRateInterlockOrderedEXT:
            return "ShadingRateInterlockOrderedEXT";
        case spv::ExecutionModeShadingRateInterlockUnorderedEXT:
            return "ShadingRateInterlockUnorderedEXT";
        case spv::ExecutionModeSharedLocalMemorySizeINTEL:
            return "SharedLocalMemorySizeINTEL";
        case spv::ExecutionModeRoundingModeRTPINTEL:
            return "RoundingModeRTPINTEL";
        case spv::ExecutionModeRoundingModeRTNINTEL:
            return "RoundingModeRTNINTEL";
        case spv::ExecutionModeFloatingPointModeALTINTEL:
            return "FloatingPointModeALTINTEL";
        case spv::ExecutionModeFloatingPointModeIEEEINTEL:
            return "FloatingPointModeIEEEINTEL";
        case spv::ExecutionModeMaxWorkgroupSizeINTEL:
            return "MaxWorkgroupSizeINTEL";
        case spv::ExecutionModeMaxWorkDimINTEL:
            return "MaxWorkDimINTEL";
        case spv::ExecutionModeNoGlobalOffsetINTEL:
            return "NoGlobalOffsetINTEL";
        case spv::ExecutionModeNumSIMDWorkitemsINTEL:
            return "NumSIMDWorkitemsINTEL";
        case spv::ExecutionModeSchedulerTargetFmaxMhzINTEL:
            return "SchedulerTargetFmaxMhzINTEL";
        case spv::ExecutionModeMaximallyReconvergesKHR:
            return "MaximallyReconvergesKHR";
        case spv::ExecutionModeFPFastMathDefault:
            return "FPFastMathDefault";
        case spv::ExecutionModeStreamingInterfaceINTEL:
            return "StreamingInterfaceINTEL";
        case spv::ExecutionModeRegisterMapInterfaceINTEL:
            return "RegisterMapInterfaceINTEL";
        case spv::ExecutionModeNamedBarrierCountINTEL:
            return "NamedBarrierCountINTEL";
        case spv::ExecutionModeMaximumRegistersINTEL:
            return "MaximumRegistersINTEL";
        case spv::ExecutionModeMaximumRegistersIdINTEL:
            return "MaximumRegistersIdINTEL";
        case spv::ExecutionModeNamedMaximumRegistersINTEL:
            return "NamedMaximumRegistersINTEL";

#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::ExecutionModeCoalescingAMDX:
            return "CoalescingAMDX";
        case spv::ExecutionModeIsApiEntryAMDX:
            return "IsApiEntryAMDX";
        case spv::ExecutionModeMaxNodeRecursionAMDX:
            return "MaxNodeRecursionAMDX";
        case spv::ExecutionModeStaticNumWorkgroupsAMDX:
            return "StaticNumWorkgroupsAMDX";
        case spv::ExecutionModeShaderIndexAMDX:
            return "ShaderIndexAMDX";
        case spv::ExecutionModeMaxNumWorkgroupsAMDX:
            return "MaxNumWorkgroupsAMDX";
        case spv::ExecutionModeSharesInputWithAMDX:
            return "SharesInputWithAMDX";
#endif
        default:
            return "Unknown Execution Mode";
    }
}

const char* string_SpvDecoration(uint32_t decoration) {
    switch (decoration) {
        case spv::DecorationRelaxedPrecision:
            return "RelaxedPrecision";
        case spv::DecorationSpecId:
            return "SpecId";
        case spv::DecorationBlock:
            return "Block";
        case spv::DecorationBufferBlock:
            return "BufferBlock";
        case spv::DecorationRowMajor:
            return "RowMajor";
        case spv::DecorationColMajor:
            return "ColMajor";
        case spv::DecorationArrayStride:
            return "ArrayStride";
        case spv::DecorationMatrixStride:
            return "MatrixStride";
        case spv::DecorationGLSLShared:
            return "GLSLShared";
        case spv::DecorationGLSLPacked:
            return "GLSLPacked";
        case spv::DecorationCPacked:
            return "CPacked";
        case spv::DecorationBuiltIn:
            return "BuiltIn";
        case spv::DecorationNoPerspective:
            return "NoPerspective";
        case spv::DecorationFlat:
            return "Flat";
        case spv::DecorationPatch:
            return "Patch";
        case spv::DecorationCentroid:
            return "Centroid";
        case spv::DecorationSample:
            return "Sample";
        case spv::DecorationInvariant:
            return "Invariant";
        case spv::DecorationRestrict:
            return "Restrict";
        case spv::DecorationAliased:
            return "Aliased";
        case spv::DecorationVolatile:
            return "Volatile";
        case spv::DecorationConstant:
            return "Constant";
        case spv::DecorationCoherent:
            return "Coherent";
        case spv::DecorationNonWritable:
            return "NonWritable";
        case spv::DecorationNonReadable:
            return "NonReadable";
        case spv::DecorationUniform:
            return "Uniform";
        case spv::DecorationUniformId:
            return "UniformId";
        case spv::DecorationSaturatedConversion:
            return "SaturatedConversion";
        case spv::DecorationStream:
            return "Stream";
        case spv::DecorationLocation:
            return "Location";
        case spv::DecorationComponent:
            return "Component";
        case spv::DecorationIndex:
            return "Index";
        case spv::DecorationBinding:
            return "Binding";
        case spv::DecorationDescriptorSet:
            return "DescriptorSet";
        case spv::DecorationOffset:
            return "Offset";
        case spv::DecorationXfbBuffer:
            return "XfbBuffer";
        case spv::DecorationXfbStride:
            return "XfbStride";
        case spv::DecorationFuncParamAttr:
            return "FuncParamAttr";
        case spv::DecorationFPRoundingMode:
            return "FPRoundingMode";
        case spv::DecorationFPFastMathMode:
            return "FPFastMathMode";
        case spv::DecorationLinkageAttributes:
            return "LinkageAttributes";
        case spv::DecorationNoContraction:
            return "NoContraction";
        case spv::DecorationInputAttachmentIndex:
            return "InputAttachmentIndex";
        case spv::DecorationAlignment:
            return "Alignment";
        case spv::DecorationMaxByteOffset:
            return "MaxByteOffset";
        case spv::DecorationAlignmentId:
            return "AlignmentId";
        case spv::DecorationMaxByteOffsetId:
            return "MaxByteOffsetId";
        case spv::DecorationNoSignedWrap:
            return "NoSignedWrap";
        case spv::DecorationNoUnsignedWrap:
            return "NoUnsignedWrap";
        case spv::DecorationWeightTextureQCOM:
            return "WeightTextureQCOM";
        case spv::DecorationBlockMatchTextureQCOM:
            return "BlockMatchTextureQCOM";
        case spv::DecorationBlockMatchSamplerQCOM:
            return "BlockMatchSamplerQCOM";
        case spv::DecorationExplicitInterpAMD:
            return "ExplicitInterpAMD";
        case spv::DecorationOverrideCoverageNV:
            return "OverrideCoverageNV";
        case spv::DecorationPassthroughNV:
            return "PassthroughNV";
        case spv::DecorationViewportRelativeNV:
            return "ViewportRelativeNV";
        case spv::DecorationSecondaryViewportRelativeNV:
            return "SecondaryViewportRelativeNV";
        case spv::DecorationPerPrimitiveEXT:
            return "PerPrimitiveEXT";
        case spv::DecorationPerViewNV:
            return "PerViewNV";
        case spv::DecorationPerTaskNV:
            return "PerTaskNV";
        case spv::DecorationPerVertexKHR:
            return "PerVertexKHR";
        case spv::DecorationNonUniform:
            return "NonUniform";
        case spv::DecorationRestrictPointer:
            return "RestrictPointer";
        case spv::DecorationAliasedPointer:
            return "AliasedPointer";
        case spv::DecorationHitObjectShaderRecordBufferNV:
            return "HitObjectShaderRecordBufferNV";
        case spv::DecorationBindlessSamplerNV:
            return "BindlessSamplerNV";
        case spv::DecorationBindlessImageNV:
            return "BindlessImageNV";
        case spv::DecorationBoundSamplerNV:
            return "BoundSamplerNV";
        case spv::DecorationBoundImageNV:
            return "BoundImageNV";
        case spv::DecorationSIMTCallINTEL:
            return "SIMTCallINTEL";
        case spv::DecorationReferencedIndirectlyINTEL:
            return "ReferencedIndirectlyINTEL";
        case spv::DecorationClobberINTEL:
            return "ClobberINTEL";
        case spv::DecorationSideEffectsINTEL:
            return "SideEffectsINTEL";
        case spv::DecorationVectorComputeVariableINTEL:
            return "VectorComputeVariableINTEL";
        case spv::DecorationFuncParamIOKindINTEL:
            return "FuncParamIOKindINTEL";
        case spv::DecorationVectorComputeFunctionINTEL:
            return "VectorComputeFunctionINTEL";
        case spv::DecorationStackCallINTEL:
            return "StackCallINTEL";
        case spv::DecorationGlobalVariableOffsetINTEL:
            return "GlobalVariableOffsetINTEL";
        case spv::DecorationCounterBuffer:
            return "CounterBuffer";
        case spv::DecorationUserSemantic:
            return "UserSemantic";
        case spv::DecorationUserTypeGOOGLE:
            return "UserTypeGOOGLE";
        case spv::DecorationFunctionRoundingModeINTEL:
            return "FunctionRoundingModeINTEL";
        case spv::DecorationFunctionDenormModeINTEL:
            return "FunctionDenormModeINTEL";
        case spv::DecorationRegisterINTEL:
            return "RegisterINTEL";
        case spv::DecorationMemoryINTEL:
            return "MemoryINTEL";
        case spv::DecorationNumbanksINTEL:
            return "NumbanksINTEL";
        case spv::DecorationBankwidthINTEL:
            return "BankwidthINTEL";
        case spv::DecorationMaxPrivateCopiesINTEL:
            return "MaxPrivateCopiesINTEL";
        case spv::DecorationSinglepumpINTEL:
            return "SinglepumpINTEL";
        case spv::DecorationDoublepumpINTEL:
            return "DoublepumpINTEL";
        case spv::DecorationMaxReplicatesINTEL:
            return "MaxReplicatesINTEL";
        case spv::DecorationSimpleDualPortINTEL:
            return "SimpleDualPortINTEL";
        case spv::DecorationMergeINTEL:
            return "MergeINTEL";
        case spv::DecorationBankBitsINTEL:
            return "BankBitsINTEL";
        case spv::DecorationForcePow2DepthINTEL:
            return "ForcePow2DepthINTEL";
        case spv::DecorationStridesizeINTEL:
            return "StridesizeINTEL";
        case spv::DecorationWordsizeINTEL:
            return "WordsizeINTEL";
        case spv::DecorationTrueDualPortINTEL:
            return "TrueDualPortINTEL";
        case spv::DecorationBurstCoalesceINTEL:
            return "BurstCoalesceINTEL";
        case spv::DecorationCacheSizeINTEL:
            return "CacheSizeINTEL";
        case spv::DecorationDontStaticallyCoalesceINTEL:
            return "DontStaticallyCoalesceINTEL";
        case spv::DecorationPrefetchINTEL:
            return "PrefetchINTEL";
        case spv::DecorationStallEnableINTEL:
            return "StallEnableINTEL";
        case spv::DecorationFuseLoopsInFunctionINTEL:
            return "FuseLoopsInFunctionINTEL";
        case spv::DecorationMathOpDSPModeINTEL:
            return "MathOpDSPModeINTEL";
        case spv::DecorationAliasScopeINTEL:
            return "AliasScopeINTEL";
        case spv::DecorationNoAliasINTEL:
            return "NoAliasINTEL";
        case spv::DecorationInitiationIntervalINTEL:
            return "InitiationIntervalINTEL";
        case spv::DecorationMaxConcurrencyINTEL:
            return "MaxConcurrencyINTEL";
        case spv::DecorationPipelineEnableINTEL:
            return "PipelineEnableINTEL";
        case spv::DecorationBufferLocationINTEL:
            return "BufferLocationINTEL";
        case spv::DecorationIOPipeStorageINTEL:
            return "IOPipeStorageINTEL";
        case spv::DecorationFunctionFloatingPointModeINTEL:
            return "FunctionFloatingPointModeINTEL";
        case spv::DecorationSingleElementVectorINTEL:
            return "SingleElementVectorINTEL";
        case spv::DecorationVectorComputeCallableFunctionINTEL:
            return "VectorComputeCallableFunctionINTEL";
        case spv::DecorationMediaBlockIOINTEL:
            return "MediaBlockIOINTEL";
        case spv::DecorationStallFreeINTEL:
            return "StallFreeINTEL";
        case spv::DecorationFPMaxErrorDecorationINTEL:
            return "FPMaxErrorDecorationINTEL";
        case spv::DecorationLatencyControlLabelINTEL:
            return "LatencyControlLabelINTEL";
        case spv::DecorationLatencyControlConstraintINTEL:
            return "LatencyControlConstraintINTEL";
        case spv::DecorationConduitKernelArgumentINTEL:
            return "ConduitKernelArgumentINTEL";
        case spv::DecorationRegisterMapKernelArgumentINTEL:
            return "RegisterMapKernelArgumentINTEL";
        case spv::DecorationMMHostInterfaceAddressWidthINTEL:
            return "MMHostInterfaceAddressWidthINTEL";
        case spv::DecorationMMHostInterfaceDataWidthINTEL:
            return "MMHostInterfaceDataWidthINTEL";
        case spv::DecorationMMHostInterfaceLatencyINTEL:
            return "MMHostInterfaceLatencyINTEL";
        case spv::DecorationMMHostInterfaceReadWriteModeINTEL:
            return "MMHostInterfaceReadWriteModeINTEL";
        case spv::DecorationMMHostInterfaceMaxBurstINTEL:
            return "MMHostInterfaceMaxBurstINTEL";
        case spv::DecorationMMHostInterfaceWaitRequestINTEL:
            return "MMHostInterfaceWaitRequestINTEL";
        case spv::DecorationStableKernelArgumentINTEL:
            return "StableKernelArgumentINTEL";
        case spv::DecorationHostAccessINTEL:
            return "HostAccessINTEL";
        case spv::DecorationInitModeINTEL:
            return "InitModeINTEL";
        case spv::DecorationImplementInRegisterMapINTEL:
            return "ImplementInRegisterMapINTEL";
        case spv::DecorationCacheControlLoadINTEL:
            return "CacheControlLoadINTEL";
        case spv::DecorationCacheControlStoreINTEL:
            return "CacheControlStoreINTEL";

#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::DecorationNodeSharesPayloadLimitsWithAMDX:
            return "NodeSharesPayloadLimitsWithAMDX";
        case spv::DecorationNodeMaxPayloadsAMDX:
            return "NodeMaxPayloadsAMDX";
        case spv::DecorationTrackFinishWritingAMDX:
            return "TrackFinishWritingAMDX";
        case spv::DecorationPayloadNodeNameAMDX:
            return "PayloadNodeNameAMDX";
        case spv::DecorationPayloadNodeBaseIndexAMDX:
            return "PayloadNodeBaseIndexAMDX";
        case spv::DecorationPayloadNodeSparseArrayAMDX:
            return "PayloadNodeSparseArrayAMDX";
        case spv::DecorationPayloadNodeArraySizeAMDX:
            return "PayloadNodeArraySizeAMDX";
        case spv::DecorationPayloadDispatchIndirectAMDX:
            return "PayloadDispatchIndirectAMDX";
#endif
        default:
            return "Unknown Decoration";
    }
}

const char* string_SpvBuiltIn(uint32_t built_in) {
    switch (built_in) {
        case spv::BuiltInPosition:
            return "Position";
        case spv::BuiltInPointSize:
            return "PointSize";
        case spv::BuiltInClipDistance:
            return "ClipDistance";
        case spv::BuiltInCullDistance:
            return "CullDistance";
        case spv::BuiltInVertexId:
            return "VertexId";
        case spv::BuiltInInstanceId:
            return "InstanceId";
        case spv::BuiltInPrimitiveId:
            return "PrimitiveId";
        case spv::BuiltInInvocationId:
            return "InvocationId";
        case spv::BuiltInLayer:
            return "Layer";
        case spv::BuiltInViewportIndex:
            return "ViewportIndex";
        case spv::BuiltInTessLevelOuter:
            return "TessLevelOuter";
        case spv::BuiltInTessLevelInner:
            return "TessLevelInner";
        case spv::BuiltInTessCoord:
            return "TessCoord";
        case spv::BuiltInPatchVertices:
            return "PatchVertices";
        case spv::BuiltInFragCoord:
            return "FragCoord";
        case spv::BuiltInPointCoord:
            return "PointCoord";
        case spv::BuiltInFrontFacing:
            return "FrontFacing";
        case spv::BuiltInSampleId:
            return "SampleId";
        case spv::BuiltInSamplePosition:
            return "SamplePosition";
        case spv::BuiltInSampleMask:
            return "SampleMask";
        case spv::BuiltInFragDepth:
            return "FragDepth";
        case spv::BuiltInHelperInvocation:
            return "HelperInvocation";
        case spv::BuiltInNumWorkgroups:
            return "NumWorkgroups";
        case spv::BuiltInWorkgroupSize:
            return "WorkgroupSize";
        case spv::BuiltInWorkgroupId:
            return "WorkgroupId";
        case spv::BuiltInLocalInvocationId:
            return "LocalInvocationId";
        case spv::BuiltInGlobalInvocationId:
            return "GlobalInvocationId";
        case spv::BuiltInLocalInvocationIndex:
            return "LocalInvocationIndex";
        case spv::BuiltInWorkDim:
            return "WorkDim";
        case spv::BuiltInGlobalSize:
            return "GlobalSize";
        case spv::BuiltInEnqueuedWorkgroupSize:
            return "EnqueuedWorkgroupSize";
        case spv::BuiltInGlobalOffset:
            return "GlobalOffset";
        case spv::BuiltInGlobalLinearId:
            return "GlobalLinearId";
        case spv::BuiltInSubgroupSize:
            return "SubgroupSize";
        case spv::BuiltInSubgroupMaxSize:
            return "SubgroupMaxSize";
        case spv::BuiltInNumSubgroups:
            return "NumSubgroups";
        case spv::BuiltInNumEnqueuedSubgroups:
            return "NumEnqueuedSubgroups";
        case spv::BuiltInSubgroupId:
            return "SubgroupId";
        case spv::BuiltInSubgroupLocalInvocationId:
            return "SubgroupLocalInvocationId";
        case spv::BuiltInVertexIndex:
            return "VertexIndex";
        case spv::BuiltInInstanceIndex:
            return "InstanceIndex";
        case spv::BuiltInCoreIDARM:
            return "CoreIDARM";
        case spv::BuiltInCoreCountARM:
            return "CoreCountARM";
        case spv::BuiltInCoreMaxIDARM:
            return "CoreMaxIDARM";
        case spv::BuiltInWarpIDARM:
            return "WarpIDARM";
        case spv::BuiltInWarpMaxIDARM:
            return "WarpMaxIDARM";
        case spv::BuiltInSubgroupEqMask:
            return "SubgroupEqMask";
        case spv::BuiltInSubgroupGeMask:
            return "SubgroupGeMask";
        case spv::BuiltInSubgroupGtMask:
            return "SubgroupGtMask";
        case spv::BuiltInSubgroupLeMask:
            return "SubgroupLeMask";
        case spv::BuiltInSubgroupLtMask:
            return "SubgroupLtMask";
        case spv::BuiltInBaseVertex:
            return "BaseVertex";
        case spv::BuiltInBaseInstance:
            return "BaseInstance";
        case spv::BuiltInDrawIndex:
            return "DrawIndex";
        case spv::BuiltInPrimitiveShadingRateKHR:
            return "PrimitiveShadingRateKHR";
        case spv::BuiltInDeviceIndex:
            return "DeviceIndex";
        case spv::BuiltInViewIndex:
            return "ViewIndex";
        case spv::BuiltInShadingRateKHR:
            return "ShadingRateKHR";
        case spv::BuiltInBaryCoordNoPerspAMD:
            return "BaryCoordNoPerspAMD";
        case spv::BuiltInBaryCoordNoPerspCentroidAMD:
            return "BaryCoordNoPerspCentroidAMD";
        case spv::BuiltInBaryCoordNoPerspSampleAMD:
            return "BaryCoordNoPerspSampleAMD";
        case spv::BuiltInBaryCoordSmoothAMD:
            return "BaryCoordSmoothAMD";
        case spv::BuiltInBaryCoordSmoothCentroidAMD:
            return "BaryCoordSmoothCentroidAMD";
        case spv::BuiltInBaryCoordSmoothSampleAMD:
            return "BaryCoordSmoothSampleAMD";
        case spv::BuiltInBaryCoordPullModelAMD:
            return "BaryCoordPullModelAMD";
        case spv::BuiltInFragStencilRefEXT:
            return "FragStencilRefEXT";
        case spv::BuiltInViewportMaskNV:
            return "ViewportMaskNV";
        case spv::BuiltInSecondaryPositionNV:
            return "SecondaryPositionNV";
        case spv::BuiltInSecondaryViewportMaskNV:
            return "SecondaryViewportMaskNV";
        case spv::BuiltInPositionPerViewNV:
            return "PositionPerViewNV";
        case spv::BuiltInViewportMaskPerViewNV:
            return "ViewportMaskPerViewNV";
        case spv::BuiltInFullyCoveredEXT:
            return "FullyCoveredEXT";
        case spv::BuiltInTaskCountNV:
            return "TaskCountNV";
        case spv::BuiltInPrimitiveCountNV:
            return "PrimitiveCountNV";
        case spv::BuiltInPrimitiveIndicesNV:
            return "PrimitiveIndicesNV";
        case spv::BuiltInClipDistancePerViewNV:
            return "ClipDistancePerViewNV";
        case spv::BuiltInCullDistancePerViewNV:
            return "CullDistancePerViewNV";
        case spv::BuiltInLayerPerViewNV:
            return "LayerPerViewNV";
        case spv::BuiltInMeshViewCountNV:
            return "MeshViewCountNV";
        case spv::BuiltInMeshViewIndicesNV:
            return "MeshViewIndicesNV";
        case spv::BuiltInBaryCoordKHR:
            return "BaryCoordKHR";
        case spv::BuiltInBaryCoordNoPerspKHR:
            return "BaryCoordNoPerspKHR";
        case spv::BuiltInFragSizeEXT:
            return "FragSizeEXT";
        case spv::BuiltInFragInvocationCountEXT:
            return "FragInvocationCountEXT";
        case spv::BuiltInPrimitivePointIndicesEXT:
            return "PrimitivePointIndicesEXT";
        case spv::BuiltInPrimitiveLineIndicesEXT:
            return "PrimitiveLineIndicesEXT";
        case spv::BuiltInPrimitiveTriangleIndicesEXT:
            return "PrimitiveTriangleIndicesEXT";
        case spv::BuiltInCullPrimitiveEXT:
            return "CullPrimitiveEXT";
        case spv::BuiltInLaunchIdKHR:
            return "LaunchIdKHR";
        case spv::BuiltInLaunchSizeKHR:
            return "LaunchSizeKHR";
        case spv::BuiltInWorldRayOriginKHR:
            return "WorldRayOriginKHR";
        case spv::BuiltInWorldRayDirectionKHR:
            return "WorldRayDirectionKHR";
        case spv::BuiltInObjectRayOriginKHR:
            return "ObjectRayOriginKHR";
        case spv::BuiltInObjectRayDirectionKHR:
            return "ObjectRayDirectionKHR";
        case spv::BuiltInRayTminKHR:
            return "RayTminKHR";
        case spv::BuiltInRayTmaxKHR:
            return "RayTmaxKHR";
        case spv::BuiltInInstanceCustomIndexKHR:
            return "InstanceCustomIndexKHR";
        case spv::BuiltInObjectToWorldKHR:
            return "ObjectToWorldKHR";
        case spv::BuiltInWorldToObjectKHR:
            return "WorldToObjectKHR";
        case spv::BuiltInHitTNV:
            return "HitTNV";
        case spv::BuiltInHitKindKHR:
            return "HitKindKHR";
        case spv::BuiltInCurrentRayTimeNV:
            return "CurrentRayTimeNV";
        case spv::BuiltInHitTriangleVertexPositionsKHR:
            return "HitTriangleVertexPositionsKHR";
        case spv::BuiltInHitMicroTriangleVertexPositionsNV:
            return "HitMicroTriangleVertexPositionsNV";
        case spv::BuiltInHitMicroTriangleVertexBarycentricsNV:
            return "HitMicroTriangleVertexBarycentricsNV";
        case spv::BuiltInIncomingRayFlagsKHR:
            return "IncomingRayFlagsKHR";
        case spv::BuiltInRayGeometryIndexKHR:
            return "RayGeometryIndexKHR";
        case spv::BuiltInHitIsSphereNV:
            return "HitIsSphereNV";
        case spv::BuiltInHitIsLSSNV:
            return "HitIsLSSNV";
        case spv::BuiltInHitSpherePositionNV:
            return "HitSpherePositionNV";
        case spv::BuiltInWarpsPerSMNV:
            return "WarpsPerSMNV";
        case spv::BuiltInSMCountNV:
            return "SMCountNV";
        case spv::BuiltInWarpIDNV:
            return "WarpIDNV";
        case spv::BuiltInSMIDNV:
            return "SMIDNV";
        case spv::BuiltInHitLSSPositionsNV:
            return "HitLSSPositionsNV";
        case spv::BuiltInHitKindFrontFacingMicroTriangleNV:
            return "HitKindFrontFacingMicroTriangleNV";
        case spv::BuiltInHitKindBackFacingMicroTriangleNV:
            return "HitKindBackFacingMicroTriangleNV";
        case spv::BuiltInHitSphereRadiusNV:
            return "HitSphereRadiusNV";
        case spv::BuiltInHitLSSRadiiNV:
            return "HitLSSRadiiNV";
        case spv::BuiltInClusterIDNV:
            return "ClusterIDNV";
        case spv::BuiltInCullMaskKHR:
            return "CullMaskKHR";

#ifdef VK_ENABLE_BETA_EXTENSIONS
        case spv::BuiltInRemainingRecursionLevelsAMDX:
            return "RemainingRecursionLevelsAMDX";
        case spv::BuiltInShaderIndexAMDX:
            return "ShaderIndexAMDX";
#endif
        default:
            return "Unknown BuiltIn";
    }
}

const char* string_SpvDim(uint32_t dim) {
    switch (dim) {
        case spv::Dim1D:
            return "1D";
        case spv::Dim2D:
            return "2D";
        case spv::Dim3D:
            return "3D";
        case spv::DimCube:
            return "Cube";
        case spv::DimRect:
            return "Rect";
        case spv::DimBuffer:
            return "Buffer";
        case spv::DimSubpassData:
            return "SubpassData";
        case spv::DimTileImageDataEXT:
            return "TileImageDataEXT";

        default:
            return "Unknown Dim";
    }
}

static const char* string_SpvCooperativeMatrixOperandsMask(spv::CooperativeMatrixOperandsMask mask) {
    switch (mask) {
        case spv::CooperativeMatrixOperandsMaskNone:
            return "NoneKHR";
        case spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask:
            return "MatrixASignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask:
            return "MatrixBSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask:
            return "MatrixCSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask:
            return "MatrixResultSignedComponentsKHR";
        case spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask:
            return "SaturatingAccumulationKHR";

        default:
            return "Unknown CooperativeMatrixOperandsMask";
    }
}

std::string string_SpvCooperativeMatrixOperands(uint32_t mask) {
    std::string ret;
    while (mask) {
        if (mask & 1) {
            if (!ret.empty()) ret.append("|");
            ret.append(string_SpvCooperativeMatrixOperandsMask(static_cast<spv::CooperativeMatrixOperandsMask>(1U << mask)));
        }
        mask >>= 1;
    }
    if (ret.empty()) ret.append("CooperativeMatrixOperandsMask(0)");
    return ret;
}

const OperandInfo& GetOperandInfo(uint32_t opcode) {
    static const vvl::unordered_map<uint32_t, OperandInfo> kOperandTable{
        // clang-format off
        {spv::OpNop, {{}}},
        {spv::OpUndef, {{}}},
        {spv::OpSourceContinued, {{OperandKind::LiteralString}}},
        {spv::OpSource, {{OperandKind::ValueEnum, OperandKind::Literal, OperandKind::Id, OperandKind::LiteralString}}},
        {spv::OpSourceExtension, {{OperandKind::LiteralString}}},
        {spv::OpName, {{OperandKind::Id, OperandKind::LiteralString}}},
        {spv::OpMemberName, {{OperandKind::Id, OperandKind::Literal, OperandKind::LiteralString}}},
        {spv::OpString, {{OperandKind::LiteralString}}},
        {spv::OpLine, {{OperandKind::Id, OperandKind::Literal, OperandKind::Literal}}},
        {spv::OpExtension, {{OperandKind::LiteralString}}},
        {spv::OpExtInstImport, {{OperandKind::LiteralString}}},
        {spv::OpExtInst, {{OperandKind::Id, OperandKind::Literal, OperandKind::Id}}},
        {spv::OpMemoryModel, {{OperandKind::ValueEnum, OperandKind::ValueEnum}}},
        {spv::OpEntryPoint, {{OperandKind::ValueEnum, OperandKind::Id, OperandKind::LiteralString, OperandKind::Id}}},
        {spv::OpExecutionMode, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpCapability, {{OperandKind::ValueEnum}}},
        {spv::OpTypeVoid, {{}}},
        {spv::OpTypeBool, {{}}},
        {spv::OpTypeInt, {{OperandKind::Literal, OperandKind::Literal}}},
        {spv::OpTypeFloat, {{OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpTypeVector, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpTypeMatrix, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpTypeImage, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Literal, OperandKind::Literal, OperandKind::Literal, OperandKind::Literal, OperandKind::ValueEnum, OperandKind::ValueEnum}}},
        {spv::OpTypeSampler, {{}}},
        {spv::OpTypeSampledImage, {{OperandKind::Id}}},
        {spv::OpTypeArray, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeRuntimeArray, {{OperandKind::Id}}},
        {spv::OpTypeStruct, {{OperandKind::Id}}},
        {spv::OpTypePointer, {{OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpTypeFunction, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeForwardPointer, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpConstantTrue, {{}}},
        {spv::OpConstantFalse, {{}}},
        {spv::OpConstant, {{OperandKind::Literal}}},
        {spv::OpConstantComposite, {{OperandKind::Id}}},
        {spv::OpConstantNull, {{}}},
        {spv::OpSpecConstantTrue, {{}}},
        {spv::OpSpecConstantFalse, {{}}},
        {spv::OpSpecConstant, {{OperandKind::Literal}}},
        {spv::OpSpecConstantComposite, {{OperandKind::Id}}},
        {spv::OpSpecConstantOp, {{OperandKind::Literal}}},
        {spv::OpFunction, {{OperandKind::BitEnum, OperandKind::Id}}},
        {spv::OpFunctionParameter, {{}}},
        {spv::OpFunctionEnd, {{}}},
        {spv::OpFunctionCall, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVariable, {{OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpImageTexelPointer, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpLoad, {{OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpStore, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCopyMemory, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCopyMemorySized, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpAccessChain, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpInBoundsAccessChain, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrAccessChain, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpArrayLength, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpInBoundsPtrAccessChain, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpDecorate, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpMemberDecorate, {{OperandKind::Id, OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpDecorationGroup, {{}}},
        {spv::OpGroupDecorate, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupMemberDecorate, {{OperandKind::Id, OperandKind::Composite}}},
        {spv::OpVectorExtractDynamic, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorInsertDynamic, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorShuffle, {{OperandKind::Id, OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCompositeConstruct, {{OperandKind::Id}}},
        {spv::OpCompositeExtract, {{OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCompositeInsert, {{OperandKind::Id, OperandKind::Id, OperandKind::Literal}}},
        {spv::OpCopyObject, {{OperandKind::Id}}},
        {spv::OpTranspose, {{OperandKind::Id}}},
        {spv::OpSampledImage, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageSampleImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSampleProjDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageFetch, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageDrefGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageRead, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageWrite, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImage, {{OperandKind::Id}}},
        {spv::OpImageQuerySizeLod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageQuerySize, {{OperandKind::Id}}},
        {spv::OpImageQueryLod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageQueryLevels, {{OperandKind::Id}}},
        {spv::OpImageQuerySamples, {{OperandKind::Id}}},
        {spv::OpConvertFToU, {{OperandKind::Id}}},
        {spv::OpConvertFToS, {{OperandKind::Id}}},
        {spv::OpConvertSToF, {{OperandKind::Id}}},
        {spv::OpConvertUToF, {{OperandKind::Id}}},
        {spv::OpUConvert, {{OperandKind::Id}}},
        {spv::OpSConvert, {{OperandKind::Id}}},
        {spv::OpFConvert, {{OperandKind::Id}}},
        {spv::OpQuantizeToF16, {{OperandKind::Id}}},
        {spv::OpConvertPtrToU, {{OperandKind::Id}}},
        {spv::OpConvertUToPtr, {{OperandKind::Id}}},
        {spv::OpBitcast, {{OperandKind::Id}}},
        {spv::OpSNegate, {{OperandKind::Id}}},
        {spv::OpFNegate, {{OperandKind::Id}}},
        {spv::OpIAdd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFAdd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISub, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFSub, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIMul, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFMul, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFDiv, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSRem, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFRem, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFMod, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorTimesScalar, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesScalar, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpVectorTimesMatrix, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesVector, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpMatrixTimesMatrix, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpOuterProduct, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAddCarry, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISubBorrow, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMulExtended, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSMulExtended, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAny, {{OperandKind::Id}}},
        {spv::OpAll, {{OperandKind::Id}}},
        {spv::OpIsNan, {{OperandKind::Id}}},
        {spv::OpIsInf, {{OperandKind::Id}}},
        {spv::OpLogicalEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalOr, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalAnd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpLogicalNot, {{OperandKind::Id}}},
        {spv::OpSelect, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpIEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpINotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpULessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpULessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpSLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordLessThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordGreaterThan, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordLessThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFOrdGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFUnordGreaterThanEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftRightLogical, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftRightArithmetic, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpShiftLeftLogical, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseOr, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseXor, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitwiseAnd, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpNot, {{OperandKind::Id}}},
        {spv::OpBitFieldInsert, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitFieldSExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitFieldUExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpBitReverse, {{OperandKind::Id}}},
        {spv::OpBitCount, {{OperandKind::Id}}},
        {spv::OpDPdx, {{OperandKind::Id}}},
        {spv::OpDPdy, {{OperandKind::Id}}},
        {spv::OpFwidth, {{OperandKind::Id}}},
        {spv::OpDPdxFine, {{OperandKind::Id}}},
        {spv::OpDPdyFine, {{OperandKind::Id}}},
        {spv::OpFwidthFine, {{OperandKind::Id}}},
        {spv::OpDPdxCoarse, {{OperandKind::Id}}},
        {spv::OpDPdyCoarse, {{OperandKind::Id}}},
        {spv::OpFwidthCoarse, {{OperandKind::Id}}},
        {spv::OpEmitVertex, {{}}},
        {spv::OpEndPrimitive, {{}}},
        {spv::OpEmitStreamVertex, {{OperandKind::Id}}},
        {spv::OpEndStreamPrimitive, {{OperandKind::Id}}},
        {spv::OpControlBarrier, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpMemoryBarrier, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicLoad, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicStore, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicExchange, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicCompareExchange, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIIncrement, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIDecrement, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicIAdd, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicISub, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicSMin, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicUMin, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicSMax, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicUMax, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicAnd, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicOr, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicXor, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpPhi, {{OperandKind::Composite}}},
        {spv::OpLoopMerge, {{OperandKind::Label, OperandKind::Label, OperandKind::BitEnum}}},
        {spv::OpSelectionMerge, {{OperandKind::Label, OperandKind::BitEnum}}},
        {spv::OpLabel, {{}}},
        {spv::OpBranch, {{OperandKind::Label}}},
        {spv::OpBranchConditional, {{OperandKind::Id, OperandKind::Label, OperandKind::Label, OperandKind::Literal}}},
        {spv::OpSwitch, {{OperandKind::Id, OperandKind::Label, OperandKind::Label}}},
        {spv::OpKill, {{}}},
        {spv::OpReturn, {{}}},
        {spv::OpReturnValue, {{OperandKind::Id}}},
        {spv::OpUnreachable, {{}}},
        {spv::OpGroupAll, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupAny, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupIAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpImageSparseSampleImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjDrefImplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseSampleProjDrefExplicitLod, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseFetch, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseDrefGather, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpImageSparseTexelsResident, {{OperandKind::Id}}},
        {spv::OpNoLine, {{}}},
        {spv::OpImageSparseRead, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpSizeOf, {{OperandKind::Id}}},
        {spv::OpModuleProcessed, {{OperandKind::LiteralString}}},
        {spv::OpExecutionModeId, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpDecorateId, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpGroupNonUniformElect, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformAll, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformAny, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformAllEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBroadcastFirst, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformInverseBallot, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotBitExtract, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotBitCount, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotFindLSB, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBallotFindMSB, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffle, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleXor, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleUp, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformShuffleDown, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformIAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFAdd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformIMul, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMul, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformSMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformUMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMin, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformSMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformUMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformFMax, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseAnd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseOr, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformBitwiseXor, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalAnd, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalOr, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformLogicalXor, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadBroadcast, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadSwap, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCopyLogical, {{OperandKind::Id}}},
        {spv::OpPtrEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrNotEqual, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpPtrDiff, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpColorAttachmentReadEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDepthAttachmentReadEXT, {{OperandKind::Id}}},
        {spv::OpStencilAttachmentReadEXT, {{OperandKind::Id}}},
        {spv::OpTerminateInvocation, {{}}},
        {spv::OpSubgroupBallotKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupFirstInvocationKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupAllKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupAnyKHR, {{OperandKind::Id}}},
        {spv::OpSubgroupAllEqualKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformRotateKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupReadInvocationKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpExtInstWithForwardRefsKHR, {{OperandKind::Id, OperandKind::Literal, OperandKind::Id}}},
        {spv::OpTraceRayKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpExecuteCallableKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpConvertUToAccelerationStructureKHR, {{OperandKind::Id}}},
        {spv::OpIgnoreIntersectionKHR, {{}}},
        {spv::OpTerminateRayKHR, {{}}},
        {spv::OpSDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpUDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSUDot, {{OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpUDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpSUDotAccSat, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpTypeCooperativeMatrixKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixMulAddKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixLengthKHR, {{OperandKind::Id}}},
        {spv::OpConstantCompositeReplicateEXT, {{OperandKind::Id}}},
        {spv::OpSpecConstantCompositeReplicateEXT, {{OperandKind::Id}}},
        {spv::OpCompositeConstructReplicateEXT, {{OperandKind::Id}}},
        {spv::OpTypeRayQueryKHR, {{}}},
        {spv::OpRayQueryInitializeKHR, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryTerminateKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGenerateIntersectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryConfirmIntersectionKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryProceedKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTypeKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageSampleWeightedQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBoxFilterQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchWindowSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchWindowSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchGatherSSDQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpImageBlockMatchGatherSADQCOM, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupIAddNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFAddNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMinNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupUMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupSMaxNonUniformAMD, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpFragmentMaskFetchAMD, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFragmentFetchAMD, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpReadClockKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadAllKHR, {{OperandKind::Id}}},
        {spv::OpGroupNonUniformQuadAnyKHR, {{OperandKind::Id}}},
        {spv::OpHitObjectRecordHitMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitWithIndexMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetWorldToObjectNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectToWorldNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayDirectionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetObjectRayOriginNV, {{OperandKind::Id}}},
        {spv::OpHitObjectTraceRayMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetShaderRecordBufferHandleNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetShaderBindingTableRecordIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectRecordEmptyNV, {{OperandKind::Id}}},
        {spv::OpHitObjectTraceRayNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordHitWithIndexNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectRecordMissNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectExecuteShaderNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetCurrentTimeNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetAttributesNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetHitKindNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetPrimitiveIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetGeometryIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceIdNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetInstanceCustomIndexNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayDirectionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetWorldRayOriginNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMaxNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetRayTMinNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsEmptyNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsHitNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsMissNV, {{OperandKind::Id}}},
        {spv::OpReorderThreadWithHitObjectNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpReorderThreadWithHintNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeHitObjectNV, {{}}},
        {spv::OpImageSampleFootprintNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpTypeCooperativeVectorNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorMatrixMulNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeVectorOuterProductAccumulateNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorReduceSumAccumulateNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorMatrixMulAddNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixConvertNV, {{OperandKind::Id}}},
        {spv::OpEmitMeshTasksEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSetMeshOutputsEXT, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpGroupNonUniformPartitionNV, {{OperandKind::Id}}},
        {spv::OpWritePackedPrimitiveIndices4x8NV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpFetchMicroTriangleVertexPositionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpFetchMicroTriangleVertexBarycentricNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeVectorLoadNV, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeVectorStoreNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpReportIntersectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIgnoreIntersectionNV, {{}}},
        {spv::OpTerminateRayNV, {{}}},
        {spv::OpTraceNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTraceMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTraceRayMotionNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeAccelerationStructureKHR, {{}}},
        {spv::OpExecuteCallableNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetClusterIdNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetClusterIdNV, {{OperandKind::Id}}},
        {spv::OpTypeCooperativeMatrixNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixMulAddNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLengthNV, {{OperandKind::Id}}},
        {spv::OpBeginInvocationInterlockEXT, {{}}},
        {spv::OpEndInvocationInterlockEXT, {{}}},
        {spv::OpCooperativeMatrixReduceNV, {{OperandKind::Id, OperandKind::BitEnum, OperandKind::Id}}},
        {spv::OpCooperativeMatrixLoadTensorNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixStoreTensorNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum, OperandKind::BitEnum}}},
        {spv::OpCooperativeMatrixPerElementOpNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeTensorLayoutNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTypeTensorViewNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpCreateTensorLayoutNV, {{}}},
        {spv::OpTensorLayoutSetDimensionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetStrideNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSliceNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetClampValueNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCreateTensorViewNV, {{}}},
        {spv::OpTensorViewSetDimensionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorViewSetStrideNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDemoteToHelperInvocation, {{}}},
        {spv::OpIsHelperInvocationEXT, {{}}},
        {spv::OpTensorViewSetClipNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpTensorLayoutSetBlockSizeNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpCooperativeMatrixTransposeNV, {{OperandKind::Id}}},
        {spv::OpConvertUToImageNV, {{OperandKind::Id}}},
        {spv::OpConvertUToSamplerNV, {{OperandKind::Id}}},
        {spv::OpConvertImageToUNV, {{OperandKind::Id}}},
        {spv::OpConvertSamplerToUNV, {{OperandKind::Id}}},
        {spv::OpConvertUToSampledImageNV, {{OperandKind::Id}}},
        {spv::OpConvertSampledImageToUNV, {{OperandKind::Id}}},
        {spv::OpSamplerImageAddressingModeNV, {{OperandKind::Literal}}},
        {spv::OpRawAccessChainNV, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpRayQueryGetIntersectionSpherePositionNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionSphereRadiusNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSPositionsNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSRadiiNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionLSSHitValueNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpHitObjectGetSpherePositionNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetSphereRadiusNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetLSSPositionsNV, {{OperandKind::Id}}},
        {spv::OpHitObjectGetLSSRadiiNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsSphereHitNV, {{OperandKind::Id}}},
        {spv::OpHitObjectIsLSSHitNV, {{OperandKind::Id}}},
        {spv::OpRayQueryIsSphereHitNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryIsLSSHitNV, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUCountLeadingZerosINTEL, {{OperandKind::Id}}},
        {spv::OpUCountTrailingZerosINTEL, {{OperandKind::Id}}},
        {spv::OpAbsISubINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAbsUSubINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAddSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAddSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAverageINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAverageINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIAverageRoundedINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUAverageRoundedINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpISubSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUSubSatINTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpIMul32x16INTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpUMul32x16INTEL, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpConstantFunctionPointerINTEL, {{OperandKind::Id}}},
        {spv::OpFunctionPointerCallINTEL, {{OperandKind::Id}}},
        {spv::OpAtomicFMinEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicFMaxEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpAssumeTrueKHR, {{OperandKind::Id}}},
        {spv::OpExpectKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpDecorateString, {{OperandKind::Id, OperandKind::ValueEnum}}},
        {spv::OpMemberDecorateString, {{OperandKind::Id, OperandKind::Literal, OperandKind::ValueEnum}}},
        {spv::OpRayQueryGetRayTMinKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetRayFlagsKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionTKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceCustomIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceIdKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionGeometryIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionPrimitiveIndexKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionBarycentricsKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionFrontFaceKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectRayDirectionKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectRayOriginKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetWorldRayDirectionKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetWorldRayOriginKHR, {{OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionObjectToWorldKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpRayQueryGetIntersectionWorldToObjectKHR, {{OperandKind::Id, OperandKind::Id}}},
        {spv::OpAtomicFAddEXT, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpArithmeticFenceEXT, {{OperandKind::Id}}},
        {spv::OpSubgroupBlockPrefetchINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpSubgroup2DBlockLoadINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockLoadTransformINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockLoadTransposeINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockPrefetchINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroup2DBlockStoreINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id}}},
        {spv::OpSubgroupMatrixMultiplyAccumulateINTEL, {{OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::Id, OperandKind::BitEnum}}},
        {spv::OpGroupIMulKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupFMulKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseAndKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseOrKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupBitwiseXorKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalAndKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalOrKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
        {spv::OpGroupLogicalXorKHR, {{OperandKind::Id, OperandKind::ValueEnum, OperandKind::Id}}},
    };  // clang-format on

    auto info = kOperandTable.find(opcode);
    if (info != kOperandTable.end()) {
        return info->second;
    }
    return kOperandTable.find(spv::OpNop)->second;
}

// NOLINTEND
