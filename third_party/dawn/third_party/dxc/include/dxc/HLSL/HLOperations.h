///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLOperations.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implentation of High Level DXIL operations.                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "llvm/IR/IRBuilder.h"
#include <string>

namespace llvm {
class Argument;
template <typename T> class ArrayRef;
class AttributeSet;
class CallInst;
class Function;
class FunctionType;
class Module;
class StringRef;
class Type;
class Value;
} // namespace llvm

namespace hlsl {

enum class HLOpcodeGroup {
  NotHL,
  HLExtIntrinsic,
  HLIntrinsic,
  HLCast,
  HLInit,
  HLBinOp,
  HLUnOp,
  HLSubscript,
  HLMatLoadStore,
  HLSelect,
  HLCreateHandle,
  // FIXME: Change the way these groups are being proliferated/used for each
  // new generated op.
  // Suggestion: Add an opcode and reuse CreateHandle/AnnotateHandle groups, and
  // add an IndexHandle group.
  HLCreateNodeOutputHandle,
  HLIndexNodeHandle,
  HLCreateNodeInputRecordHandle,
  HLAnnotateHandle,
  HLAnnotateNodeHandle,
  HLAnnotateNodeRecordHandle,
  NumOfHLOps
};

enum class HLBinaryOpcode {
  Invalid,
  Mul,
  Div,
  Rem,
  Add,
  Sub,
  Shl,
  Shr,
  LT,
  GT,
  LE,
  GE,
  EQ,
  NE,
  And,
  Xor,
  Or,
  LAnd,
  LOr,
  UDiv,
  URem,
  UShr,
  ULT,
  UGT,
  ULE,
  UGE,
  NumOfBO,
};

enum class HLUnaryOpcode {
  Invalid,
  PostInc,
  PostDec,
  PreInc,
  PreDec,
  Plus,
  Minus,
  Not,
  LNot,
  NumOfUO,
};

enum class HLSubscriptOpcode {
  DefaultSubscript,
  ColMatSubscript,
  RowMatSubscript,
  ColMatElement,
  RowMatElement,
  DoubleSubscript,
  CBufferSubscript,
  VectorSubscript, // Only for bool vector, other vector type will use GEP
                   // directly.
};

enum class HLCastOpcode {
  DefaultCast,
  UnsignedUnsignedCast,
  FromUnsignedCast,
  ToUnsignedCast,
  ColMatrixToVecCast,
  RowMatrixToVecCast,
  ColMatrixToRowMatrix,
  RowMatrixToColMatrix,
  HandleToResCast,
  HandleToNodeOutputCast,
  NodeOutputToHandleCast,
  HandleToNodeRecordCast,
  NodeRecordToHandleCast,
};

enum class HLMatLoadStoreOpcode {
  ColMatLoad,
  ColMatStore,
  RowMatLoad,
  RowMatStore,
};

extern const char *const HLPrefix;

HLOpcodeGroup GetHLOpcodeGroup(llvm::Function *F);
HLOpcodeGroup GetHLOpcodeGroupByName(const llvm::Function *F);
llvm::StringRef GetHLOpcodeGroupNameByAttr(llvm::Function *F);
llvm::StringRef GetHLLowerStrategy(llvm::Function *F);
unsigned GetHLOpcode(const llvm::CallInst *CI);
unsigned GetRowMajorOpcode(HLOpcodeGroup group, unsigned opcode);
void SetHLLowerStrategy(llvm::Function *F, llvm::StringRef S);

void SetHLWaveSensitive(llvm::Function *F);
bool IsHLWaveSensitive(llvm::Function *F);

// For intrinsic opcode.
unsigned GetUnsignedOpcode(unsigned opcode);
// For HLBinaryOpcode.
bool HasUnsignedOpcode(HLBinaryOpcode opcode);
HLBinaryOpcode GetUnsignedOpcode(HLBinaryOpcode opcode);

llvm::StringRef GetHLOpcodeGroupName(HLOpcodeGroup op);

namespace HLOperandIndex {
// Opcode parameter.
const unsigned kOpcodeIdx = 0;

// Used to initialize values that have no valid index in the HL overload.
const unsigned kInvalidIdx = UINT32_MAX;

// Matrix store.
const unsigned kMatStoreDstPtrOpIdx = 1;
const unsigned kMatStoreValOpIdx = 2;

// Matrix load.
const unsigned kMatLoadPtrOpIdx = 1;

// Normal subscipts.
const unsigned kSubscriptObjectOpIdx = 1;
const unsigned kSubscriptIndexOpIdx = 2;

// Double subscripts.
const unsigned kDoubleSubscriptMipLevelOpIdx = 3;

// Matrix subscripts.
const unsigned kMatSubscriptMatOpIdx = 1;
const unsigned kMatSubscriptSubOpIdx = 2;

// Matrix init.
const unsigned kMatArrayInitMatOpIdx = 1;
const unsigned kMatArrayInitFirstArgOpIdx = 2;

// Array Init.
const unsigned kArrayInitPtrOpIdx = 1;
const unsigned kArrayInitFirstArgOpIdx = 2;

// Normal Init.
const unsigned kInitFirstArgOpIdx = 1;

// Unary operators.
const unsigned kUnaryOpSrc0Idx = 1;

// Binary operators.
const unsigned kBinaryOpSrc0Idx = 1;
const unsigned kBinaryOpSrc1Idx = 2;

// Trinary operators.
const unsigned kTrinaryOpSrc0Idx = 1;
const unsigned kTrinaryOpSrc1Idx = 2;
const unsigned kTrinaryOpSrc2Idx = 3;

// Interlocked.
const unsigned kInterlockedDestOpIndex = 1;
const unsigned kInterlockedValueOpIndex = 2;
const unsigned kInterlockedOriginalValueOpIndex = 3;

// Interlocked method
const unsigned kInterlockedMethodValueOpIndex = 3;

// InterlockedCompareExchange.
const unsigned kInterlockedCmpDestOpIndex = 1;
const unsigned kInterlockedCmpCompareValueOpIndex = 2;
const unsigned kInterlockedCmpValueOpIndex = 3;
const unsigned kInterlockedCmpOriginalValueOpIndex = 4;

// Lerp.
const unsigned kLerpOpXIdx = 1;
const unsigned kLerpOpYIdx = 2;
const unsigned kLerpOpSIdx = 3;

// ProcessTessFactorIsoline.
const unsigned kProcessTessFactorRawDetailFactor = 1;
const unsigned kProcessTessFactorRawDensityFactor = 2;
const unsigned kProcessTessFactorRoundedDetailFactor = 3;
const unsigned kProcessTessFactorRoundedDensityFactor = 4;

// ProcessTessFactor.
const unsigned kProcessTessFactorRawEdgeFactor = 1;
const unsigned kProcessTessFactorInsideScale = 2;
const unsigned kProcessTessFactorRoundedEdgeFactor = 3;
const unsigned kProcessTessFactorRoundedInsideFactor = 4;
const unsigned kProcessTessFactorUnRoundedInsideFactor = 5;

// Reflect.
const unsigned kReflectOpIIdx = 1;
const unsigned kReflectOpNIdx = 2;

// Refract
const unsigned kRefractOpIIdx = 1;
const unsigned kRefractOpNIdx = 2;
const unsigned kRefractOpEtaIdx = 3;

// SmoothStep.
const unsigned kSmoothStepOpMinIdx = 1;
const unsigned kSmoothStepOpMaxIdx = 2;
const unsigned kSmoothStepOpXIdx = 3;

// Clamp
const unsigned kClampOpXIdx = 1;
const unsigned kClampOpMinIdx = 2;
const unsigned kClampOpMaxIdx = 3;

// Object functions.
const unsigned kHandleOpIdx = 1;
// Store.
const unsigned kStoreOffsetOpIdx = 2;
const unsigned kStoreValOpIdx = 3;
// Load.
const unsigned kBufLoadAddrOpIdx = 2;
const unsigned kBufLoadStatusOpIdx = 3;
const unsigned kRWTexLoadStatusOpIdx = 3;
const unsigned kTexLoadOffsetOpIdx = 3;
const unsigned kTexLoadStatusOpIdx = 4;
// Load for Texture2DMS
const unsigned kTex2DMSLoadSampleIdxOpIdx = 3;
const unsigned kTex2DMSLoadOffsetOpIdx = 4;
const unsigned kTex2DMSLoadStatusOpIdx = 5;
// mips.Operator.
const unsigned kMipLoadAddrOpIdx = 3;
const unsigned kMipLoadOffsetOpIdx = 4;
const unsigned kMipLoadStatusOpIdx = 5;

// Sample.
const unsigned kSampleSamplerArgIndex = 2;
const unsigned kSampleCoordArgIndex = 3;
const unsigned kSampleOffsetArgIndex = 4;
const unsigned kSampleClampArgIndex = 5;
const unsigned kSampleStatusArgIndex = 6;

// SampleG.
const unsigned kSampleGDDXArgIndex = 4;
const unsigned kSampleGDDYArgIndex = 5;
const unsigned kSampleGOffsetArgIndex = 6;
const unsigned kSampleGClampArgIndex = 7;
const unsigned kSampleGStatusArgIndex = 8;

// SampleCmp.
const unsigned kSampleCmpCmpValArgIndex = 4;
const unsigned kSampleCmpOffsetArgIndex = 5;
const unsigned kSampleCmpClampArgIndex = 6;
const unsigned kSampleCmpStatusArgIndex = 7;

// SampleCmpBias.
const unsigned kSampleCmpBCmpValArgIndex = 4;
const unsigned kSampleCmpBBiasArgIndex = 5;
const unsigned kSampleCmpBOffsetArgIndex = 6;
const unsigned kSampleCmpBClampArgIndex = 7;
const unsigned kSampleCmpBStatusArgIndex = 8;

// SampleCmpGrad.
const unsigned kSampleCmpGCmpValArgIndex = 4;
const unsigned kSampleCmpGDDXArgIndex = 5;
const unsigned kSampleCmpGDDYArgIndex = 6;
const unsigned kSampleCmpGOffsetArgIndex = 7;
const unsigned kSampleCmpGClampArgIndex = 8;
const unsigned kSampleCmpGStatusArgIndex = 9;

// SampleBias.
const unsigned kSampleBBiasArgIndex = 4;
const unsigned kSampleBOffsetArgIndex = 5;
const unsigned kSampleBClampArgIndex = 6;
const unsigned kSampleBStatusArgIndex = 7;

// SampleLevel.
const unsigned kSampleLLevelArgIndex = 4;
const unsigned kSampleLOffsetArgIndex = 5;
const unsigned kSampleLStatusArgIndex = 6;

// SampleCmpLevel
// the rest are the same as SampleCmp
const unsigned kSampleCmpLLevelArgIndex = 5;
const unsigned kSampleCmpLOffsetArgIndex = 6;

// SampleCmpLevelZero.
const unsigned kSampleCmpLZCmpValArgIndex = 4;
const unsigned kSampleCmpLZOffsetArgIndex = 5;
const unsigned kSampleCmpLZStatusArgIndex = 6;

// Gather.
const unsigned kGatherSamplerArgIndex = 2;
const unsigned kGatherCoordArgIndex = 3;
const unsigned kGatherOffsetArgIndex = 4;
const unsigned kGatherStatusArgIndex = 5;
const unsigned kGatherSampleOffsetArgIndex = 5;
const unsigned kGatherStatusWithSampleOffsetArgIndex = 8;
const unsigned kGatherCubeStatusArgIndex = 4;

// GatherCmp.
const unsigned kGatherCmpCmpValArgIndex = 4;
const unsigned kGatherCmpOffsetArgIndex = 5;
const unsigned kGatherCmpStatusArgIndex = 6;
const unsigned kGatherCmpSampleOffsetArgIndex = 6;
const unsigned kGatherCmpStatusWithSampleOffsetArgIndex = 9;
const unsigned kGatherCmpCubeStatusArgIndex = 5;

// WriteSamplerFeedback.
const unsigned kWriteSamplerFeedbackSampledArgIndex = 2;
const unsigned kWriteSamplerFeedbackSamplerArgIndex = 3;
const unsigned kWriteSamplerFeedbackCoordArgIndex = 4;
const unsigned kWriteSamplerFeedbackBias_BiasArgIndex = 5;
const unsigned kWriteSamplerFeedbackLevel_LodArgIndex = 5;
const unsigned kWriteSamplerFeedbackGrad_DdxArgIndex = 5;
const unsigned kWriteSamplerFeedbackGrad_DdyArgIndex = 6;
const unsigned kWriteSamplerFeedback_ClampArgIndex = 5;
const unsigned kWriteSamplerFeedbackBias_ClampArgIndex = 6;
const unsigned kWriteSamplerFeedbackGrad_ClampArgIndex = 7;

// StreamAppend.
const unsigned kStreamAppendStreamOpIndex = 1;
const unsigned kStreamAppendDataOpIndex = 2;

// Append.
const unsigned kAppendValOpIndex = 2;

// Interlocked.
const unsigned kObjectInterlockedDestOpIndex = 2;
const unsigned kObjectInterlockedValueOpIndex = 3;
const unsigned kObjectInterlockedOriginalValueOpIndex = 4;

// InterlockedCompareExchange.
const unsigned kObjectInterlockedCmpDestOpIndex = 2;
const unsigned kObjectInterlockedCmpCompareValueOpIndex = 3;
const unsigned kObjectInterlockedCmpValueOpIndex = 4;
const unsigned kObjectInterlockedCmpOriginalValueOpIndex = 5;

// GetSamplePosition.
const unsigned kGetSamplePositionSampleIdxOpIndex = 2;

// GetDimensions.
const unsigned kGetDimensionsMipLevelOpIndex = 2;
const unsigned kGetDimensionsMipWidthOpIndex = 3;
const unsigned kGetDimensionsNoMipWidthOpIndex = 2;

// WaveAllEqual.
const unsigned kWaveAllEqualValueOpIdx = 1;

// CreateHandle.
const unsigned kCreateHandleResourceOpIdx = 1;
const unsigned kCreateHandleIndexOpIdx = 2; // Only for array of cbuffer.

// Annotate(Node)(Record)Handle.
const unsigned kAnnotateHandleResourcePropertiesOpIdx = 2;
const unsigned kAnnotateHandleResourceTypeOpIdx = 3;

// TraceRay.
const unsigned kTraceRayRayDescOpIdx = 7;
// kTraceRayPayloadPreOpIdx is before flattening the RayDesc
const unsigned kTraceRayPayloadPreOpIdx = 8;
// kTraceRayPayloadOpIdx is after flattening the RayDesc
const unsigned kTraceRayPayloadOpIdx = 11;
const unsigned kTraceRay_PreNumOp = 9;
const unsigned kTraceRay_NumOp = 12;

// AllocateRayQuery
const unsigned kAllocateRayQueryRayFlagsIdx = 1;
const unsigned kAllocateRayQueryRayQueryFlagsIdx = 2;

// CallShader.
const unsigned kCallShaderPayloadOpIdx = 2;

// TraceRayInline.
const unsigned kTraceRayInlineRayDescOpIdx = 5;
// kTraceRayInlinePayloadPreOpIdx is before flattening the RayDesc
const unsigned kTraceRayInlinePayloadPreOpIdx = 6;
// kTraceRayInlinePayloadOpIdx is after flattening the RayDesc
const unsigned kTraceRayInlinePayloadOpIdx = 9;

// ReportIntersection.
const unsigned kReportIntersectionAttributeOpIdx = 3;

// DispatchMesh
const unsigned kDispatchMeshOpThreadX = 1;
const unsigned kDispatchMeshOpThreadY = 2;
const unsigned kDispatchMeshOpThreadZ = 3;
const unsigned kDispatchMeshOpPayload = 4;

// Work Graph

const unsigned kIncrementOutputCountCountIdx = 2;

const unsigned kBarrierMemoryTypeFlagsOpIdx = 1;
const unsigned kBarrierSemanticFlagsOpIdx = 2;

// Node Handles
const unsigned kAllocateRecordNumRecordsIdx = 2;
const unsigned kNodeOutputMetadataIDIdx = 1;
const unsigned kIndexNodeHandleArrayIDIdx = 2;
const unsigned kNodeInputRecordMetadataIDIdx = 1;
const unsigned kNodeHandleToResCastOpIdx = 1;
const unsigned kAnnotateNodeHandleNodePropIdx = 2;
const unsigned kAnnotateNodeRecordHandleNodeRecordPropIdx = 2;

// HitObject::MakeMiss
const unsigned kHitObjectMakeMiss_NumOp = 8;
const unsigned kHitObjectMakeMiss_RayDescOpIdx = 4;

// HitObject::TraceRay
const unsigned kHitObjectTraceRay_RayDescOpIdx = 8;
// kHitObjectTraceRay_PayloadPreOpIdx is before flattening the RayDesc
const unsigned kHitObjectTraceRay_PayloadPreOpIdx = 9;
// kHitObjectTraceRay_PayloadOpIdx is after flattening the RayDesc
const unsigned kHitObjectTraceRay_PayloadOpIdx = 12;
const unsigned kHitObjectTraceRay_PreNumOp = 10;
const unsigned kHitObjectTraceRay_NumOp = 13;

// HitObject::Invoke
const unsigned kHitObjectInvoke_PayloadOpIdx = 2;

// HitObject::FromRayQuery
const unsigned kHitObjectFromRayQuery_WithAttrs_AttributeOpIdx = 4;
const unsigned kHitObjectFromRayQuery_WithAttrs_NumOp = 5;

// HitObject::GetAttributes
const unsigned kHitObjectGetAttributes_AttributeOpIdx = 2;

// Linear Algebra Operations

// MatVecMul
const unsigned kMatVecMulOutputVectorIdx = 1;
const unsigned kMatVecMulIsOutputUnsignedIdx = 2;
const unsigned kMatVecMulInputVectorIdx = 3;
const unsigned kMatVecMulIsInputUnsignedIdx = 4;
const unsigned kMatVecMulInputInterpretationIdx = 5;
const unsigned kMatVecMulMatrixBufferIdx = 6;
const unsigned kMatVecMulMatrixOffsetIdx = 7;
const unsigned kMatVecMulMatrixInterpretationIdx = 8;
const unsigned kMatVecMulMatrixMIdx = 9;
const unsigned kMatVecMulMatrixKIdx = 10;
const unsigned kMatVecMulMatrixLayoutIdx = 11;
const unsigned kMatVecMulMatrixTransposeIdx = 12;
const unsigned kMatVecMulMatrixStrideIdx = 13;

// MatVecMulAdd
const unsigned kMatVecMulAddOutputVectorIdx = 1;
const unsigned kMatVecMulAddIsOutputUnsignedIdx = 2;
const unsigned kMatVecMulAddInputVectorIdx = 3;
const unsigned kMatVecMulAddIsInputUnsignedIdx = 4;
const unsigned kMatVecMulAddInputInterpretationIdx = 5;
const unsigned kMatVecMulAddMatrixBufferIdx = 6;
const unsigned kMatVecMulAddMatrixOffsetIdx = 7;
const unsigned kMatVecMulAddMatrixInterpretationIdx = 8;
const unsigned kMatVecMulAddMatrixMIdx = 9;
const unsigned kMatVecMulAddMatrixKIdx = 10;
const unsigned kMatVecMulAddMatrixLayoutIdx = 11;
const unsigned kMatVecMulAddMatrixTransposeIdx = 12;
const unsigned kMatVecMulAddMatrixStrideIdx = 13;
const unsigned kMatVecMulAddBiasBufferIdx = 14;
const unsigned kMatVecMulAddBiasOffsetIdx = 15;
const unsigned kMatVecMulAddBiasInterpretationIdx = 16;

// OuterProductAccumulate
const unsigned kOuterProdAccInputVec1Idx = 1;
const unsigned kOuterProdAccInputVec2Idx = 2;
const unsigned kOuterProdAccMatrixIdx = 3;
const unsigned kOuterProdAccMatrixOffsetIdx = 4;
const unsigned kOuterProdAccMatrixInterpretationIdx = 5;
const unsigned kOuterProdAccMatrixLayoutIdx = 6;
const unsigned kOuterProdAccMatrixStrideIdx = 7;

// Vector Accumulate
const unsigned kVectorAccInputVecIdx = 1;
const unsigned kVectorAccMatrixIdx = 2;
const unsigned kVectorAccMatrixOffsetIdx = 3;
} // namespace HLOperandIndex

llvm::Function *GetOrCreateHLFunction(llvm::Module &M,
                                      llvm::FunctionType *funcTy,
                                      HLOpcodeGroup group, unsigned opcode);
llvm::Function *GetOrCreateHLFunction(llvm::Module &M,
                                      llvm::FunctionType *funcTy,
                                      HLOpcodeGroup group,
                                      llvm::StringRef *groupName,
                                      llvm::StringRef *fnName, unsigned opcode);

llvm::Function *GetOrCreateHLFunction(llvm::Module &M,
                                      llvm::FunctionType *funcTy,
                                      HLOpcodeGroup group, unsigned opcode,
                                      const llvm::AttributeSet &attribs);
llvm::Function *GetOrCreateHLFunction(llvm::Module &M,
                                      llvm::FunctionType *funcTy,
                                      HLOpcodeGroup group,
                                      llvm::StringRef *groupName,
                                      llvm::StringRef *fnName, unsigned opcode,
                                      const llvm::AttributeSet &attribs);

llvm::Function *GetOrCreateHLFunctionWithBody(llvm::Module &M,
                                              llvm::FunctionType *funcTy,
                                              HLOpcodeGroup group,
                                              unsigned opcode,
                                              llvm::StringRef name);

llvm::Value *callHLFunction(llvm::Module &Module, HLOpcodeGroup OpcodeGroup,
                            unsigned Opcode, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Value *> Args,
                            const llvm::AttributeSet &attribs,
                            llvm::IRBuilder<> &Builder);

llvm::Value *callHLFunction(llvm::Module &Module, HLOpcodeGroup OpcodeGroup,
                            unsigned Opcode, llvm::Type *RetTy,
                            llvm::ArrayRef<llvm::Value *> Args,
                            llvm::IRBuilder<> &Builder);

} // namespace hlsl
