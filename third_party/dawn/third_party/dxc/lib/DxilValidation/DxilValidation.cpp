///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilValidation.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file provides support for validating DXIL shaders.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilEntryProps.h"
#include "dxc/DXIL/DxilFunctionProps.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilResourceProperties.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilValidation/DxilValidation.h"
#include "dxc/HLSL/DxilGenerationPass.h"
#include "llvm/Analysis/ReducibilityAnalysis.h"

#include "dxc/HLSL/DxilPackSignatureElement.h"
#include "dxc/HLSL/DxilSignatureAllocator.h"
#include "dxc/HLSL/DxilSpanAllocator.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

#include "DxilValidationUtils.h"

#include <algorithm>
#include <deque>
#include <unordered_set>

using namespace llvm;
using std::unique_ptr;
using std::unordered_set;
using std::vector;

///////////////////////////////////////////////////////////////////////////////
// Error messages.

#include "DxilValidationImpl.inc"

namespace hlsl {

// PrintDiagnosticContext methods.
PrintDiagnosticContext::PrintDiagnosticContext(DiagnosticPrinter &printer)
    : m_Printer(printer), m_errorsFound(false), m_warningsFound(false) {}

bool PrintDiagnosticContext::HasErrors() const { return m_errorsFound; }
bool PrintDiagnosticContext::HasWarnings() const { return m_warningsFound; }
void PrintDiagnosticContext::Handle(const DiagnosticInfo &DI) {
  DI.print(m_Printer);
  switch (DI.getSeverity()) {
  case llvm::DiagnosticSeverity::DS_Error:
    m_errorsFound = true;
    break;
  case llvm::DiagnosticSeverity::DS_Warning:
    m_warningsFound = true;
    break;
  default:
    break;
  }
  m_Printer << "\n";
}

void PrintDiagnosticContext::PrintDiagnosticHandler(const DiagnosticInfo &DI,
                                                    void *Context) {
  reinterpret_cast<hlsl::PrintDiagnosticContext *>(Context)->Handle(DI);
}

struct PSExecutionInfo {
  bool SuperSampling = false;
  DXIL::SemanticKind OutputDepthKind = DXIL::SemanticKind::Invalid;
  const InterpolationMode *PositionInterpolationMode = nullptr;
};

static unsigned ValidateSignatureRowCol(Instruction *I,
                                        DxilSignatureElement &SE, Value *rowVal,
                                        Value *colVal, EntryStatus &Status,
                                        ValidationContext &ValCtx) {
  if (ConstantInt *constRow = dyn_cast<ConstantInt>(rowVal)) {
    unsigned row = constRow->getLimitedValue();
    if (row >= SE.GetRows()) {
      std::string range = std::string("0~") + std::to_string(SE.GetRows());
      ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOperandRange,
                                  {"Row", range, std::to_string(row)});
    }
  }

  if (!isa<ConstantInt>(colVal)) {
    // col must be const
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"Col", "LoadInput/StoreOutput"});
    return 0;
  }

  unsigned col = cast<ConstantInt>(colVal)->getLimitedValue();

  if (col > SE.GetCols()) {
    std::string range = std::string("0~") + std::to_string(SE.GetCols());
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOperandRange,
                                {"Col", range, std::to_string(col)});
  } else {
    if (SE.IsOutput())
      Status.outputCols[SE.GetID()] |= 1 << col;
    if (SE.IsPatchConstOrPrim())
      Status.patchConstOrPrimCols[SE.GetID()] |= 1 << col;
  }

  return col;
}

static DxilSignatureElement *
ValidateSignatureAccess(Instruction *I, DxilSignature &sig, Value *sigID,
                        Value *rowVal, Value *colVal, EntryStatus &Status,
                        ValidationContext &ValCtx) {
  if (!isa<ConstantInt>(sigID)) {
    // inputID must be const
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"SignatureID", "LoadInput/StoreOutput"});
    return nullptr;
  }

  unsigned SEIdx = cast<ConstantInt>(sigID)->getLimitedValue();
  if (sig.GetElements().size() <= SEIdx) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrOpConstRange);
    return nullptr;
  }

  DxilSignatureElement &SE = sig.GetElement(SEIdx);
  bool isOutput = sig.IsOutput();

  unsigned col = ValidateSignatureRowCol(I, SE, rowVal, colVal, Status, ValCtx);

  if (isOutput && SE.GetSemantic()->GetKind() == DXIL::SemanticKind::Position) {
    unsigned mask = Status.OutputPositionMask[SE.GetOutputStream()];
    mask |= 1 << col;
    if (SE.GetOutputStream() < DXIL::kNumOutputStreams)
      Status.OutputPositionMask[SE.GetOutputStream()] = mask;
  }
  return &SE;
}

static DxilResourceProperties GetResourceFromHandle(Value *Handle,
                                                    ValidationContext &ValCtx) {
  if (!isa<CallInst>(Handle)) {
    if (Instruction *I = dyn_cast<Instruction>(Handle))
      ValCtx.EmitInstrError(I, ValidationRule::InstrHandleNotFromCreateHandle);
    else
      ValCtx.EmitError(ValidationRule::InstrHandleNotFromCreateHandle);
    DxilResourceProperties RP;
    return RP;
  }

  DxilResourceProperties RP = ValCtx.GetResourceFromVal(Handle);
  if (RP.getResourceClass() == DXIL::ResourceClass::Invalid) {
    ValCtx.EmitInstrError(cast<CallInst>(Handle),
                          ValidationRule::InstrHandleNotFromCreateHandle);
  }

  return RP;
}

static DXIL::SamplerKind GetSamplerKind(Value *samplerHandle,
                                        ValidationContext &ValCtx) {
  DxilResourceProperties RP = GetResourceFromHandle(samplerHandle, ValCtx);

  if (RP.getResourceClass() != DXIL::ResourceClass::Sampler) {
    // must be sampler.
    return DXIL::SamplerKind::Invalid;
  }
  if (RP.Basic.SamplerCmpOrHasCounter)
    return DXIL::SamplerKind::Comparison;
  else if (RP.getResourceKind() == DXIL::ResourceKind::Invalid)
    return DXIL::SamplerKind::Invalid;
  else
    return DXIL::SamplerKind::Default;
}

static DXIL::ResourceKind
GetResourceKindAndCompTy(Value *handle, DXIL::ComponentType &CompTy,
                         DXIL::ResourceClass &ResClass,
                         ValidationContext &ValCtx) {
  CompTy = DXIL::ComponentType::Invalid;
  ResClass = DXIL::ResourceClass::Invalid;
  // TODO: validate ROV is used only in PS.

  DxilResourceProperties RP = GetResourceFromHandle(handle, ValCtx);
  ResClass = RP.getResourceClass();

  switch (ResClass) {
  case DXIL::ResourceClass::SRV:
  case DXIL::ResourceClass::UAV:
    break;
  case DXIL::ResourceClass::CBuffer:
    return DXIL::ResourceKind::CBuffer;
  case DXIL::ResourceClass::Sampler:
    return DXIL::ResourceKind::Sampler;
  default:
    // Emit invalid res class
    return DXIL::ResourceKind::Invalid;
  }
  if (!DXIL::IsStructuredBuffer(RP.getResourceKind()))
    CompTy = static_cast<DXIL::ComponentType>(RP.Typed.CompType);
  else
    CompTy = DXIL::ComponentType::Invalid;

  return RP.getResourceKind();
}

DxilFieldAnnotation *GetFieldAnnotation(Type *Ty, DxilTypeSystem &typeSys,
                                        std::deque<unsigned> &offsets) {
  unsigned CurIdx = 1;
  unsigned LastIdx = offsets.size() - 1;
  DxilStructAnnotation *StructAnnot = nullptr;

  for (; CurIdx < offsets.size(); ++CurIdx) {
    if (const StructType *EltST = dyn_cast<StructType>(Ty)) {
      if (DxilStructAnnotation *EltAnnot = typeSys.GetStructAnnotation(EltST)) {
        StructAnnot = EltAnnot;
        Ty = EltST->getElementType(offsets[CurIdx]);
        if (CurIdx == LastIdx) {
          return &StructAnnot->GetFieldAnnotation(offsets[CurIdx]);
        }
      } else {
        return nullptr;
      }
    } else if (const ArrayType *AT = dyn_cast<ArrayType>(Ty)) {
      Ty = AT->getElementType();
      StructAnnot = nullptr;
    } else {
      if (StructAnnot)
        return &StructAnnot->GetFieldAnnotation(offsets[CurIdx]);
    }
  }
  return nullptr;
}

DxilResourceProperties ValidationContext::GetResourceFromVal(Value *resVal) {
  auto it = ResPropMap.find(resVal);
  if (it != ResPropMap.end()) {
    return it->second;
  } else {
    DxilResourceProperties RP;
    return RP;
  }
}

struct ResRetUsage {
  bool x;
  bool y;
  bool z;
  bool w;
  bool status;
  ResRetUsage() : x(false), y(false), z(false), w(false), status(false) {}
};

static void CollectGetDimResRetUsage(ResRetUsage &usage, Instruction *ResRet,
                                     ValidationContext &ValCtx) {
  for (User *U : ResRet->users()) {
    if (ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(U)) {
      for (unsigned idx : EVI->getIndices()) {
        switch (idx) {
        case 0:
          usage.x = true;
          break;
        case 1:
          usage.y = true;
          break;
        case 2:
          usage.z = true;
          break;
        case 3:
          usage.w = true;
          break;
        case DXIL::kResRetStatusIndex:
          usage.status = true;
          break;
        default:
          // Emit index out of bound.
          ValCtx.EmitInstrError(EVI,
                                ValidationRule::InstrDxilStructUserOutOfBound);
          break;
        }
      }
    } else if (PHINode *PHI = dyn_cast<PHINode>(U)) {
      CollectGetDimResRetUsage(usage, PHI, ValCtx);
    } else {
      Instruction *User = cast<Instruction>(U);
      ValCtx.EmitInstrError(User, ValidationRule::InstrDxilStructUser);
    }
  }
}

static void ValidateResourceCoord(CallInst *CI, DXIL::ResourceKind resKind,
                                  ArrayRef<Value *> coords,
                                  ValidationContext &ValCtx) {
  const unsigned kMaxNumCoords = 4;
  unsigned numCoords = DxilResource::GetNumCoords(resKind);
  for (unsigned i = 0; i < kMaxNumCoords; i++) {
    if (i < numCoords) {
      if (isa<UndefValue>(coords[i])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceCoordinateMiss);
      }
    } else {
      if (!isa<UndefValue>(coords[i])) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrResourceCoordinateTooMany);
      }
    }
  }
}

static void ValidateCalcLODResourceDimensionCoord(CallInst *CI,
                                                  DXIL::ResourceKind resKind,
                                                  ArrayRef<Value *> coords,
                                                  ValidationContext &ValCtx) {
  const unsigned kMaxNumDimCoords = 3;
  unsigned numCoords = DxilResource::GetNumDimensionsForCalcLOD(resKind);
  for (unsigned i = 0; i < kMaxNumDimCoords; i++) {
    if (i < numCoords) {
      if (isa<UndefValue>(coords[i])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceCoordinateMiss);
      }
    } else {
      if (!isa<UndefValue>(coords[i])) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrResourceCoordinateTooMany);
      }
    }
  }
}

static void ValidateResourceOffset(CallInst *CI, DXIL::ResourceKind resKind,
                                   ArrayRef<Value *> offsets,
                                   ValidationContext &ValCtx) {
  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();

  unsigned numOffsets = DxilResource::GetNumOffsets(resKind);
  bool hasOffset = !isa<UndefValue>(offsets[0]);

  auto validateOffset = [&](Value *offset) {
    // 6.7 Advanced Textures allow programmable offsets
    if (pSM->IsSM67Plus())
      return;
    if (ConstantInt *cOffset = dyn_cast<ConstantInt>(offset)) {
      int offset = cOffset->getValue().getSExtValue();
      if (offset > 7 || offset < -8) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrTextureOffset);
      }
    } else {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrTextureOffset);
    }
  };

  if (hasOffset) {
    validateOffset(offsets[0]);
  }

  for (unsigned i = 1; i < offsets.size(); i++) {
    if (i < numOffsets) {
      if (hasOffset) {
        if (isa<UndefValue>(offsets[i]))
          ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetMiss);
        else
          validateOffset(offsets[i]);
      }
    } else {
      if (!isa<UndefValue>(offsets[i])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetTooMany);
      }
    }
  }
}

// Validate derivative and derivative dependent ops in CS/MS/AS
static void ValidateDerivativeOp(CallInst *CI, ValidationContext &ValCtx) {

  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
  if (pSM && (pSM->IsMS() || pSM->IsAS() || pSM->IsCS()) && !pSM->IsSM66Plus())
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::SmOpcodeInInvalidFunction,
        {"Derivatives in CS/MS/AS", "Shader Model 6.6+"});
}

static void ValidateSampleInst(CallInst *CI, Value *srvHandle,
                               Value *samplerHandle, ArrayRef<Value *> coords,
                               ArrayRef<Value *> offsets, bool IsSampleC,
                               ValidationContext &ValCtx) {
  if (!IsSampleC) {
    if (GetSamplerKind(samplerHandle, ValCtx) != DXIL::SamplerKind::Default) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSample);
    }
  } else {
    if (GetSamplerKind(samplerHandle, ValCtx) !=
        DXIL::SamplerKind::Comparison) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSampleC);
    }
  }

  DXIL::ComponentType compTy;
  DXIL::ResourceClass resClass;
  DXIL::ResourceKind resKind =
      GetResourceKindAndCompTy(srvHandle, compTy, resClass, ValCtx);
  bool isSampleCompTy = compTy == DXIL::ComponentType::F32;
  isSampleCompTy |= compTy == DXIL::ComponentType::SNormF32;
  isSampleCompTy |= compTy == DXIL::ComponentType::UNormF32;
  isSampleCompTy |= compTy == DXIL::ComponentType::F16;
  isSampleCompTy |= compTy == DXIL::ComponentType::SNormF16;
  isSampleCompTy |= compTy == DXIL::ComponentType::UNormF16;
  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
  if (pSM->IsSM67Plus() && !IsSampleC) {
    isSampleCompTy |= compTy == DXIL::ComponentType::I16;
    isSampleCompTy |= compTy == DXIL::ComponentType::U16;
    isSampleCompTy |= compTy == DXIL::ComponentType::I32;
    isSampleCompTy |= compTy == DXIL::ComponentType::U32;
  }
  if (!isSampleCompTy) {
    ValCtx.EmitInstrError(CI, ValidationRule::InstrSampleCompType);
  }

  if (resClass != DXIL::ResourceClass::SRV) {
    ValCtx.EmitInstrError(CI,
                          ValidationRule::InstrResourceClassForSamplerGather);
  }

  ValidationRule rule = ValidationRule::InstrResourceKindForSample;
  if (IsSampleC) {
    rule = ValidationRule::InstrResourceKindForSampleC;
  }

  switch (resKind) {
  case DXIL::ResourceKind::Texture1D:
  case DXIL::ResourceKind::Texture1DArray:
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture2DArray:
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::TextureCubeArray:
    break;
  case DXIL::ResourceKind::Texture3D:
    if (IsSampleC) {
      ValCtx.EmitInstrError(CI, rule);
    }
    break;
  default:
    ValCtx.EmitInstrError(CI, rule);
    return;
  }

  // Coord match resource kind.
  ValidateResourceCoord(CI, resKind, coords, ValCtx);
  // Offset match resource kind.
  ValidateResourceOffset(CI, resKind, offsets, ValCtx);
}

static void ValidateGather(CallInst *CI, Value *srvHandle, Value *samplerHandle,
                           ArrayRef<Value *> coords, ArrayRef<Value *> offsets,
                           bool IsSampleC, ValidationContext &ValCtx) {
  if (!IsSampleC) {
    if (GetSamplerKind(samplerHandle, ValCtx) != DXIL::SamplerKind::Default) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSample);
    }
  } else {
    if (GetSamplerKind(samplerHandle, ValCtx) !=
        DXIL::SamplerKind::Comparison) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSampleC);
    }
  }

  DXIL::ComponentType compTy;
  DXIL::ResourceClass resClass;
  DXIL::ResourceKind resKind =
      GetResourceKindAndCompTy(srvHandle, compTy, resClass, ValCtx);

  if (resClass != DXIL::ResourceClass::SRV) {
    ValCtx.EmitInstrError(CI,
                          ValidationRule::InstrResourceClassForSamplerGather);
    return;
  }

  // Coord match resource kind.
  ValidateResourceCoord(CI, resKind, coords, ValCtx);
  // Offset match resource kind.
  switch (resKind) {
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture2DArray: {
    bool hasOffset = !isa<UndefValue>(offsets[0]);
    if (hasOffset) {
      if (isa<UndefValue>(offsets[1])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetMiss);
      }
    }
  } break;
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::TextureCubeArray: {
    if (!isa<UndefValue>(offsets[0])) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetTooMany);
    }
    if (!isa<UndefValue>(offsets[1])) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetTooMany);
    }
  } break;
  default:
    // Invalid resource type for gather.
    ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForGather);
    return;
  }
}

static unsigned StoreValueToMask(ArrayRef<Value *> vals) {
  unsigned mask = 0;
  for (unsigned i = 0; i < 4; i++) {
    if (!isa<UndefValue>(vals[i])) {
      mask |= 1 << i;
    }
  }
  return mask;
}

static int GetCBufSize(Value *cbHandle, ValidationContext &ValCtx) {
  DxilResourceProperties RP = GetResourceFromHandle(cbHandle, ValCtx);

  if (RP.getResourceClass() != DXIL::ResourceClass::CBuffer) {
    ValCtx.EmitInstrError(cast<CallInst>(cbHandle),
                          ValidationRule::InstrCBufferClassForCBufferHandle);
    return -1;
  }

  return RP.CBufferSizeInBytes;
}

// Make sure none of the handle arguments are undef / zero-initializer,
// Also, do not accept any resource handles with invalid dxil resource
// properties
void ValidateHandleArgsForInstruction(CallInst *CI, DXIL::OpCode opcode,
                                      ValidationContext &ValCtx) {

  for (Value *op : CI->operands()) {
    const Type *pHandleTy = ValCtx.HandleTy; // This is a resource handle
    const Type *pNodeHandleTy = ValCtx.DxilMod.GetOP()->GetNodeHandleType();
    const Type *pNodeRecordHandleTy =
        ValCtx.DxilMod.GetOP()->GetNodeRecordHandleType();

    const Type *argTy = op->getType();
    if (argTy == pNodeHandleTy || argTy == pNodeRecordHandleTy ||
        argTy == pHandleTy) {

      if (isa<UndefValue>(op) || isa<ConstantAggregateZero>(op)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
      } else if (argTy == pHandleTy) {
        // GetResourceFromHandle will emit an error on an invalid handle
        GetResourceFromHandle(op, ValCtx);
      }
    }
  }
}

void ValidateHandleArgs(CallInst *CI, DXIL::OpCode opcode,
                        ValidationContext &ValCtx) {

  switch (opcode) {
    // TODO: add case DXIL::OpCode::IndexNodeRecordHandle:

  case DXIL::OpCode::AnnotateHandle:
  case DXIL::OpCode::AnnotateNodeHandle:
  case DXIL::OpCode::AnnotateNodeRecordHandle:
  case DXIL::OpCode::CreateHandleForLib:
    // TODO: add custom validation for these intrinsics
    break;

  default:
    ValidateHandleArgsForInstruction(CI, opcode, ValCtx);
    break;
  }
}

static unsigned GetNumVertices(DXIL::InputPrimitive inputPrimitive) {
  const unsigned InputPrimitiveVertexTab[] = {
      0,  // Undefined = 0,
      1,  // Point = 1,
      2,  // Line = 2,
      3,  // Triangle = 3,
      0,  // Reserved4 = 4,
      0,  // Reserved5 = 5,
      4,  // LineWithAdjacency = 6,
      6,  // TriangleWithAdjacency = 7,
      1,  // ControlPointPatch1 = 8,
      2,  // ControlPointPatch2 = 9,
      3,  // ControlPointPatch3 = 10,
      4,  // ControlPointPatch4 = 11,
      5,  // ControlPointPatch5 = 12,
      6,  // ControlPointPatch6 = 13,
      7,  // ControlPointPatch7 = 14,
      8,  // ControlPointPatch8 = 15,
      9,  // ControlPointPatch9 = 16,
      10, // ControlPointPatch10 = 17,
      11, // ControlPointPatch11 = 18,
      12, // ControlPointPatch12 = 19,
      13, // ControlPointPatch13 = 20,
      14, // ControlPointPatch14 = 21,
      15, // ControlPointPatch15 = 22,
      16, // ControlPointPatch16 = 23,
      17, // ControlPointPatch17 = 24,
      18, // ControlPointPatch18 = 25,
      19, // ControlPointPatch19 = 26,
      20, // ControlPointPatch20 = 27,
      21, // ControlPointPatch21 = 28,
      22, // ControlPointPatch22 = 29,
      23, // ControlPointPatch23 = 30,
      24, // ControlPointPatch24 = 31,
      25, // ControlPointPatch25 = 32,
      26, // ControlPointPatch26 = 33,
      27, // ControlPointPatch27 = 34,
      28, // ControlPointPatch28 = 35,
      29, // ControlPointPatch29 = 36,
      30, // ControlPointPatch30 = 37,
      31, // ControlPointPatch31 = 38,
      32, // ControlPointPatch32 = 39,
      0,  // LastEntry,
  };

  unsigned primitiveIdx = static_cast<unsigned>(inputPrimitive);
  return InputPrimitiveVertexTab[primitiveIdx];
}

static void ValidateSignatureDxilOp(CallInst *CI, DXIL::OpCode opcode,
                                    ValidationContext &ValCtx) {
  Function *F = CI->getParent()->getParent();
  DxilModule &DM = ValCtx.DxilMod;
  bool bIsPatchConstantFunc = false;
  if (!DM.HasDxilEntryProps(F)) {
    auto it = ValCtx.PatchConstantFuncMap.find(F);
    if (it == ValCtx.PatchConstantFuncMap.end()) {
      // Missing entry props.
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrSignatureOperationNotInEntry);
      return;
    }
    // Use hull entry instead of patch constant function.
    F = it->second.front();
    bIsPatchConstantFunc = true;
  }
  if (!ValCtx.HasEntryStatus(F)) {
    return;
  }

  EntryStatus &Status = ValCtx.GetEntryStatus(F);
  DxilEntryProps &EntryProps = DM.GetDxilEntryProps(F);
  DxilFunctionProps &props = EntryProps.props;
  DxilEntrySignature &S = EntryProps.sig;

  switch (opcode) {
  case DXIL::OpCode::LoadInput: {
    Value *inputID = CI->getArgOperand(DXIL::OperandIndex::kLoadInputIDOpIdx);
    DxilSignature &inputSig = S.InputSignature;
    Value *row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    ValidateSignatureAccess(CI, inputSig, inputID, row, col, Status, ValCtx);

    // Check vertexID in ps/vs. and none array input.
    Value *vertexID =
        CI->getArgOperand(DXIL::OperandIndex::kLoadInputVertexIDOpIdx);
    bool usedVertexID = vertexID && !isa<UndefValue>(vertexID);
    if (props.IsVS() || props.IsPS()) {
      if (usedVertexID) {
        // use vertexID in VS/PS input.
        ValCtx.EmitInstrError(CI, ValidationRule::SmOperand);
        return;
      }
    } else {
      if (ConstantInt *cVertexID = dyn_cast<ConstantInt>(vertexID)) {
        int immVertexID = cVertexID->getValue().getLimitedValue();
        if (cVertexID->getValue().isNegative()) {
          immVertexID = cVertexID->getValue().getSExtValue();
        }
        const int low = 0;
        int high = 0;
        if (props.IsGS()) {
          DXIL::InputPrimitive inputPrimitive =
              props.ShaderProps.GS.inputPrimitive;
          high = GetNumVertices(inputPrimitive);
        } else if (props.IsDS()) {
          high = props.ShaderProps.DS.inputControlPoints;
        } else if (props.IsHS()) {
          high = props.ShaderProps.HS.inputControlPoints;
        } else {
          ValCtx.EmitInstrFormatError(CI,
                                      ValidationRule::SmOpcodeInInvalidFunction,
                                      {"LoadInput", "VS/HS/DS/GS/PS"});
        }
        if (immVertexID < low || immVertexID >= high) {
          std::string range = std::to_string(low) + "~" + std::to_string(high);
          ValCtx.EmitInstrFormatError(
              CI, ValidationRule::InstrOperandRange,
              {"VertexID", range, std::to_string(immVertexID)});
        }
      }
    }
  } break;
  case DXIL::OpCode::DomainLocation: {
    Value *colValue =
        CI->getArgOperand(DXIL::OperandIndex::kDomainLocationColOpIdx);
    if (!isa<ConstantInt>(colValue)) {
      // col must be const
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                  {"Col", "DomainLocation"});
    } else {
      unsigned col = cast<ConstantInt>(colValue)->getLimitedValue();
      if (col >= Status.domainLocSize) {
        ValCtx.EmitInstrError(CI, ValidationRule::SmDomainLocationIdxOOB);
      }
    }
  } break;
  case DXIL::OpCode::StoreOutput:
  case DXIL::OpCode::StoreVertexOutput:
  case DXIL::OpCode::StorePrimitiveOutput: {
    Value *outputID =
        CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
    DxilSignature &outputSig = opcode == DXIL::OpCode::StorePrimitiveOutput
                                   ? S.PatchConstOrPrimSignature
                                   : S.OutputSignature;
    Value *row = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
    Value *col = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
    ValidateSignatureAccess(CI, outputSig, outputID, row, col, Status, ValCtx);
  } break;
  case DXIL::OpCode::OutputControlPointID: {
    // Only used in hull shader.
    Function *func = CI->getParent()->getParent();
    // Make sure this is inside hs shader entry function.
    if (!(props.IsHS() && F == func)) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"OutputControlPointID", "hull function"});
    }
  } break;
  case DXIL::OpCode::LoadOutputControlPoint: {
    // Only used in patch constant function.
    Function *func = CI->getParent()->getParent();
    if (ValCtx.entryFuncCallSet.count(func) > 0) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"LoadOutputControlPoint", "PatchConstant function"});
    }
    Value *outputID =
        CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
    DxilSignature &outputSig = S.OutputSignature;
    Value *row = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
    Value *col = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
    ValidateSignatureAccess(CI, outputSig, outputID, row, col, Status, ValCtx);
  } break;
  case DXIL::OpCode::StorePatchConstant: {
    // Only used in patch constant function.
    Function *func = CI->getParent()->getParent();
    if (!bIsPatchConstantFunc) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"StorePatchConstant", "PatchConstant function"});
    } else {
      auto &hullShaders = ValCtx.PatchConstantFuncMap[func];
      for (Function *F : hullShaders) {
        EntryStatus &Status = ValCtx.GetEntryStatus(F);
        DxilEntryProps &EntryProps = DM.GetDxilEntryProps(F);
        DxilEntrySignature &S = EntryProps.sig;
        Value *outputID =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
        DxilSignature &outputSig = S.PatchConstOrPrimSignature;
        Value *row =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
        Value *col =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
        ValidateSignatureAccess(CI, outputSig, outputID, row, col, Status,
                                ValCtx);
      }
    }
  } break;
  case DXIL::OpCode::Coverage:
    Status.m_bCoverageIn = true;
    break;
  case DXIL::OpCode::InnerCoverage:
    Status.m_bInnerCoverageIn = true;
    break;
  case DXIL::OpCode::ViewID:
    Status.hasViewID = true;
    break;
  case DXIL::OpCode::EvalCentroid:
  case DXIL::OpCode::EvalSampleIndex:
  case DXIL::OpCode::EvalSnapped: {
    // Eval* share same operand index with load input.
    Value *inputID = CI->getArgOperand(DXIL::OperandIndex::kLoadInputIDOpIdx);
    DxilSignature &inputSig = S.InputSignature;
    Value *row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    DxilSignatureElement *pSE = ValidateSignatureAccess(
        CI, inputSig, inputID, row, col, Status, ValCtx);
    if (pSE) {
      switch (pSE->GetInterpolationMode()->GetKind()) {
      case DXIL::InterpolationMode::Linear:
      case DXIL::InterpolationMode::LinearNoperspective:
      case DXIL::InterpolationMode::LinearCentroid:
      case DXIL::InterpolationMode::LinearNoperspectiveCentroid:
      case DXIL::InterpolationMode::LinearSample:
      case DXIL::InterpolationMode::LinearNoperspectiveSample:
        break;
      default:
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrEvalInterpolationMode, {pSE->GetName()});
        break;
      }
      if (pSE->GetSemantic()->GetKind() == DXIL::SemanticKind::Position) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrCannotPullPosition,
            {ValCtx.DxilMod.GetShaderModel()->GetName()});
      }
    }
  } break;
  case DXIL::OpCode::AttributeAtVertex: {
    Value *Attribute = CI->getArgOperand(DXIL::OperandIndex::kBinarySrc0OpIdx);
    DxilSignature &inputSig = S.InputSignature;
    Value *row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    DxilSignatureElement *pSE = ValidateSignatureAccess(
        CI, inputSig, Attribute, row, col, Status, ValCtx);
    if (pSE && pSE->GetInterpolationMode()->GetKind() !=
                   hlsl::InterpolationMode::Kind::Constant) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::InstrAttributeAtVertexNoInterpolation,
          {pSE->GetName()});
    }
  } break;
  case DXIL::OpCode::CutStream:
  case DXIL::OpCode::EmitThenCutStream:
  case DXIL::OpCode::EmitStream: {
    if (props.IsGS()) {
      auto &GS = props.ShaderProps.GS;
      unsigned streamMask = 0;
      for (size_t i = 0; i < _countof(GS.streamPrimitiveTopologies); ++i) {
        if (GS.streamPrimitiveTopologies[i] !=
            DXIL::PrimitiveTopology::Undefined) {
          streamMask |= 1 << i;
        }
      }
      Value *streamID =
          CI->getArgOperand(DXIL::OperandIndex::kStreamEmitCutIDOpIdx);
      if (ConstantInt *cStreamID = dyn_cast<ConstantInt>(streamID)) {
        int immStreamID = cStreamID->getValue().getLimitedValue();
        if (cStreamID->getValue().isNegative() || immStreamID >= 4) {
          ValCtx.EmitInstrFormatError(
              CI, ValidationRule::InstrOperandRange,
              {"StreamID", "0~4", std::to_string(immStreamID)});
        } else {
          unsigned immMask = 1 << immStreamID;
          if ((streamMask & immMask) == 0) {
            std::string range;
            for (unsigned i = 0; i < 4; i++) {
              if (streamMask & (1 << i)) {
                range += std::to_string(i) + " ";
              }
            }
            ValCtx.EmitInstrFormatError(
                CI, ValidationRule::InstrOperandRange,
                {"StreamID", range, std::to_string(immStreamID)});
          }
        }

      } else {
        ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                    {"StreamID", "Emit/CutStream"});
      }
    } else {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"Emit/CutStream", "Geometry shader"});
    }
  } break;
  case DXIL::OpCode::EmitIndices: {
    if (!props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"EmitIndices", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::SetMeshOutputCounts: {
    if (!props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"SetMeshOutputCounts", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::GetMeshPayload: {
    if (!props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"GetMeshPayload", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::DispatchMesh: {
    if (!props.IsAS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"DispatchMesh", "Amplification shader"});
    }
  } break;
  default:
    break;
  }

  if (Status.m_bCoverageIn && Status.m_bInnerCoverageIn) {
    ValCtx.EmitInstrError(CI, ValidationRule::SmPSCoverageAndInnerCoverage);
  }
}

static void ValidateImmOperandForMathDxilOp(CallInst *CI, DXIL::OpCode opcode,
                                            ValidationContext &ValCtx) {
  switch (opcode) {
  // Imm input value validation.
  case DXIL::OpCode::Asin: {
    DxilInst_Asin I(CI);
    if (ConstantFP *imm = dyn_cast<ConstantFP>(I.get_value())) {
      if (imm->getValueAPF().isInfinity()) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoIndefiniteAsin);
      }
    }
  } break;
  case DXIL::OpCode::Acos: {
    DxilInst_Acos I(CI);
    if (ConstantFP *imm = dyn_cast<ConstantFP>(I.get_value())) {
      if (imm->getValueAPF().isInfinity()) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoIndefiniteAcos);
      }
    }
  } break;
  case DXIL::OpCode::Log: {
    DxilInst_Log I(CI);
    if (ConstantFP *imm = dyn_cast<ConstantFP>(I.get_value())) {
      if (imm->getValueAPF().isInfinity()) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoIndefiniteLog);
      }
    }
  } break;
  case DXIL::OpCode::DerivFineX:
  case DXIL::OpCode::DerivFineY:
  case DXIL::OpCode::DerivCoarseX:
  case DXIL::OpCode::DerivCoarseY: {
    Value *V = CI->getArgOperand(DXIL::OperandIndex::kUnarySrc0OpIdx);
    if (ConstantFP *imm = dyn_cast<ConstantFP>(V)) {
      if (imm->getValueAPF().isInfinity()) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoIndefiniteDsxy);
      }
    }
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  default:
    break;
  }
}

// Validate the type-defined mask compared to the store value mask which
// indicates which parts were defined returns true if caller should continue
// validation
static bool ValidateStorageMasks(Instruction *I, DXIL::OpCode opcode,
                                 ConstantInt *mask, unsigned stValMask,
                                 bool isTyped, ValidationContext &ValCtx) {
  if (!mask) {
    // Mask for buffer store should be immediate.
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"Mask", hlsl::OP::GetOpCodeName(opcode)});
    return false;
  }

  unsigned uMask = mask->getLimitedValue();
  if (isTyped && uMask != 0xf) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrWriteMaskForTypedUAVStore);
  }

  // write mask must be contiguous (.x .xy .xyz or .xyzw)
  if (!((uMask == 0xf) || (uMask == 0x7) || (uMask == 0x3) || (uMask == 0x1))) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrWriteMaskGapForUAV);
  }

  // If a bit is set in the uMask (expected values) that isn't set in stValMask
  // (user provided values) then the user failed to define some of the output
  // values.
  if (uMask & ~stValMask)
    ValCtx.EmitInstrError(I, ValidationRule::InstrUndefinedValueForUAVStore);
  else if (uMask != stValMask)
    ValCtx.EmitInstrFormatError(
        I, ValidationRule::InstrWriteMaskMatchValueForUAVStore,
        {std::to_string(uMask), std::to_string(stValMask)});

  return true;
}

static void ValidateResourceDxilOp(CallInst *CI, DXIL::OpCode opcode,
                                   ValidationContext &ValCtx) {
  switch (opcode) {
  case DXIL::OpCode::GetDimensions: {
    DxilInst_GetDimensions getDim(CI);
    Value *handle = getDim.get_handle();
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(handle, compTy, resClass, ValCtx);

    // Check the result component use.
    ResRetUsage usage;
    CollectGetDimResRetUsage(usage, CI, ValCtx);

    // Mip level only for texture.
    switch (resKind) {
    case DXIL::ResourceKind::Texture1D:
      if (usage.y) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"y", "Texture1D"});
      }
      if (usage.z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture1D"});
      }
      break;
    case DXIL::ResourceKind::Texture1DArray:
      if (usage.z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture1DArray"});
      }
      break;
    case DXIL::ResourceKind::Texture2D:
      if (usage.z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture2D"});
      }
      break;
    case DXIL::ResourceKind::Texture2DArray:
      break;
    case DXIL::ResourceKind::Texture2DMS:
      if (usage.z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture2DMS"});
      }
      break;
    case DXIL::ResourceKind::Texture2DMSArray:
      break;
    case DXIL::ResourceKind::Texture3D:
      break;
    case DXIL::ResourceKind::TextureCube:
      if (usage.z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "TextureCube"});
      }
      break;
    case DXIL::ResourceKind::TextureCubeArray:
      break;
    case DXIL::ResourceKind::StructuredBuffer:
    case DXIL::ResourceKind::RawBuffer:
    case DXIL::ResourceKind::TypedBuffer:
    case DXIL::ResourceKind::TBuffer: {
      Value *mip = getDim.get_mipLevel();
      if (!isa<UndefValue>(mip)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrMipLevelForGetDimension);
      }
      if (resKind != DXIL::ResourceKind::Invalid) {
        if (usage.y || usage.z || usage.w) {
          ValCtx.EmitInstrFormatError(
              CI, ValidationRule::InstrUndefResultForGetDimension,
              {"invalid", "resource"});
        }
      }
    } break;
    default: {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForGetDim);
    } break;
    }

    if (usage.status) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::InstrUndefResultForGetDimension,
          {"invalid", "resource"});
    }
  } break;
  case DXIL::OpCode::CalculateLOD: {
    DxilInst_CalculateLOD lod(CI);
    Value *samplerHandle = lod.get_sampler();
    DXIL::SamplerKind samplerKind = GetSamplerKind(samplerHandle, ValCtx);
    if (samplerKind != DXIL::SamplerKind::Default) {
      // After SM68, Comparison is supported.
      if (!ValCtx.DxilMod.GetShaderModel()->IsSM68Plus() ||
          samplerKind != DXIL::SamplerKind::Comparison)
        ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForLOD);
    }
    Value *handle = lod.get_handle();
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(handle, compTy, resClass, ValCtx);
    if (resClass != DXIL::ResourceClass::SRV) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrResourceClassForSamplerGather);
      return;
    }
    // Coord match resource.
    ValidateCalcLODResourceDimensionCoord(
        CI, resKind, {lod.get_coord0(), lod.get_coord1(), lod.get_coord2()},
        ValCtx);

    switch (resKind) {
    case DXIL::ResourceKind::Texture1D:
    case DXIL::ResourceKind::Texture1DArray:
    case DXIL::ResourceKind::Texture2D:
    case DXIL::ResourceKind::Texture2DArray:
    case DXIL::ResourceKind::Texture3D:
    case DXIL::ResourceKind::TextureCube:
    case DXIL::ResourceKind::TextureCubeArray:
      break;
    default:
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForCalcLOD);
      break;
    }

    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::TextureGather: {
    DxilInst_TextureGather gather(CI);
    ValidateGather(CI, gather.get_srv(), gather.get_sampler(),
                   {gather.get_coord0(), gather.get_coord1(),
                    gather.get_coord2(), gather.get_coord3()},
                   {gather.get_offset0(), gather.get_offset1()},
                   /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::TextureGatherCmp: {
    DxilInst_TextureGatherCmp gather(CI);
    ValidateGather(CI, gather.get_srv(), gather.get_sampler(),
                   {gather.get_coord0(), gather.get_coord1(),
                    gather.get_coord2(), gather.get_coord3()},
                   {gather.get_offset0(), gather.get_offset1()},
                   /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::Sample: {
    DxilInst_Sample sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmp: {
    DxilInst_SampleCmp sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpLevel: {
    // sampler must be comparison mode.
    DxilInst_SampleCmpLevel sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpLevelZero: {
    // sampler must be comparison mode.
    DxilInst_SampleCmpLevelZero sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleBias: {
    DxilInst_SampleBias sample(CI);
    Value *bias = sample.get_bias();
    if (ConstantFP *cBias = dyn_cast<ConstantFP>(bias)) {
      float fBias = cBias->getValueAPF().convertToFloat();
      if (fBias < DXIL::kMinMipLodBias || fBias > DXIL::kMaxMipLodBias) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrImmBiasForSampleB,
            {std::to_string(DXIL::kMinMipLodBias),
             std::to_string(DXIL::kMaxMipLodBias),
             std::to_string(cBias->getValueAPF().convertToFloat())});
      }
    }

    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpBias: {
    DxilInst_SampleCmpBias sample(CI);
    Value *bias = sample.get_bias();
    if (ConstantFP *cBias = dyn_cast<ConstantFP>(bias)) {
      float fBias = cBias->getValueAPF().convertToFloat();
      if (fBias < DXIL::kMinMipLodBias || fBias > DXIL::kMaxMipLodBias) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrImmBiasForSampleB,
            {std::to_string(DXIL::kMinMipLodBias),
             std::to_string(DXIL::kMaxMipLodBias),
             std::to_string(cBias->getValueAPF().convertToFloat())});
      }
    }

    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleGrad: {
    DxilInst_SampleGrad sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpGrad: {
    DxilInst_SampleCmpGrad sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleLevel: {
    DxilInst_SampleLevel sample(CI);
    ValidateSampleInst(
        CI, sample.get_srv(), sample.get_sampler(),
        {sample.get_coord0(), sample.get_coord1(), sample.get_coord2(),
         sample.get_coord3()},
        {sample.get_offset0(), sample.get_offset1(), sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::CheckAccessFullyMapped: {
    Value *Src = CI->getArgOperand(DXIL::OperandIndex::kUnarySrc0OpIdx);
    ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(Src);
    if (!EVI) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrCheckAccessFullyMapped);
    } else {
      Value *V = EVI->getOperand(0);
      bool isLegal = EVI->getNumIndices() == 1 &&
                     EVI->getIndices()[0] == DXIL::kResRetStatusIndex &&
                     ValCtx.DxilMod.GetOP()->IsResRetType(V->getType());
      if (!isLegal) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCheckAccessFullyMapped);
      }
    }
  } break;
  case DXIL::OpCode::BufferStore: {
    DxilInst_BufferStore bufSt(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(bufSt.get_uav(), compTy, resClass, ValCtx);

    if (resClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);
    }

    ConstantInt *mask = dyn_cast<ConstantInt>(bufSt.get_mask());
    unsigned stValMask =
        StoreValueToMask({bufSt.get_value0(), bufSt.get_value1(),
                          bufSt.get_value2(), bufSt.get_value3()});

    if (!ValidateStorageMasks(CI, opcode, mask, stValMask,
                              resKind == DXIL::ResourceKind::TypedBuffer ||
                                  resKind == DXIL::ResourceKind::TBuffer,
                              ValCtx))
      return;
    Value *offset = bufSt.get_coord1();

    switch (resKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::TypedBuffer:
    case DXIL::ResourceKind::TBuffer:
      if (!isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrCoordinateCountForStructBuf);
      }
      break;
    default:
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrResourceKindForBufferLoadStore);
      break;
    }

  } break;
  case DXIL::OpCode::TextureStore: {
    DxilInst_TextureStore texSt(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(texSt.get_srv(), compTy, resClass, ValCtx);

    if (resClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);
    }

    ConstantInt *mask = dyn_cast<ConstantInt>(texSt.get_mask());
    unsigned stValMask =
        StoreValueToMask({texSt.get_value0(), texSt.get_value1(),
                          texSt.get_value2(), texSt.get_value3()});

    if (!ValidateStorageMasks(CI, opcode, mask, stValMask, true /*isTyped*/,
                              ValCtx))
      return;

    switch (resKind) {
    case DXIL::ResourceKind::Texture1D:
    case DXIL::ResourceKind::Texture1DArray:
    case DXIL::ResourceKind::Texture2D:
    case DXIL::ResourceKind::Texture2DArray:
    case DXIL::ResourceKind::Texture2DMS:
    case DXIL::ResourceKind::Texture2DMSArray:
    case DXIL::ResourceKind::Texture3D:
      break;
    default:
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrResourceKindForTextureStore);
      break;
    }
  } break;
  case DXIL::OpCode::BufferLoad: {
    DxilInst_BufferLoad bufLd(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(bufLd.get_srv(), compTy, resClass, ValCtx);

    if (resClass != DXIL::ResourceClass::SRV &&
        resClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);
    }

    Value *offset = bufLd.get_wot();

    switch (resKind) {
    case DXIL::ResourceKind::RawBuffer:
    case DXIL::ResourceKind::TypedBuffer:
    case DXIL::ResourceKind::TBuffer:
      if (!isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrCoordinateCountForStructBuf);
      }
      break;
    default:
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrResourceKindForBufferLoadStore);
      break;
    }

  } break;
  case DXIL::OpCode::TextureLoad: {
    DxilInst_TextureLoad texLd(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(texLd.get_srv(), compTy, resClass, ValCtx);

    Value *mipLevel = texLd.get_mipLevelOrSampleCount();

    if (resClass == DXIL::ResourceClass::UAV) {
      bool noOffset = isa<UndefValue>(texLd.get_offset0());
      noOffset &= isa<UndefValue>(texLd.get_offset1());
      noOffset &= isa<UndefValue>(texLd.get_offset2());
      if (!noOffset) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrOffsetOnUAVLoad);
      }
      if (!isa<UndefValue>(mipLevel)) {
        if (resKind != DXIL::ResourceKind::Texture2DMS &&
            resKind != DXIL::ResourceKind::Texture2DMSArray)
          ValCtx.EmitInstrError(CI, ValidationRule::InstrMipOnUAVLoad);
      }
    } else {
      if (resClass != DXIL::ResourceClass::SRV) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);
      }
    }

    switch (resKind) {
    case DXIL::ResourceKind::Texture1D:
    case DXIL::ResourceKind::Texture1DArray:
    case DXIL::ResourceKind::Texture2D:
    case DXIL::ResourceKind::Texture2DArray:
    case DXIL::ResourceKind::Texture3D:
      break;
    case DXIL::ResourceKind::Texture2DMS:
    case DXIL::ResourceKind::Texture2DMSArray: {
      if (isa<UndefValue>(mipLevel)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrSampleIndexForLoad2DMS);
      }
    } break;
    default:
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrResourceKindForTextureLoad);
      return;
    }

    ValidateResourceOffset(
        CI, resKind,
        {texLd.get_offset0(), texLd.get_offset1(), texLd.get_offset2()},
        ValCtx);
  } break;
  case DXIL::OpCode::CBufferLoad: {
    DxilInst_CBufferLoad CBLoad(CI);
    Value *regIndex = CBLoad.get_byteOffset();
    if (ConstantInt *cIndex = dyn_cast<ConstantInt>(regIndex)) {
      int offset = cIndex->getLimitedValue();
      int size = GetCBufSize(CBLoad.get_handle(), ValCtx);
      if (size > 0 && offset >= size) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCBufferOutOfBound);
      }
    }
  } break;
  case DXIL::OpCode::CBufferLoadLegacy: {
    DxilInst_CBufferLoadLegacy CBLoad(CI);
    Value *regIndex = CBLoad.get_regIndex();
    if (ConstantInt *cIndex = dyn_cast<ConstantInt>(regIndex)) {
      int offset = cIndex->getLimitedValue() * 16; // 16 bytes align
      int size = GetCBufSize(CBLoad.get_handle(), ValCtx);
      if (size > 0 && offset >= size) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCBufferOutOfBound);
      }
    }
  } break;
  case DXIL::OpCode::RawBufferLoad: {
    if (!ValCtx.DxilMod.GetShaderModel()->IsSM63Plus()) {
      Type *Ty = OP::GetOverloadType(DXIL::OpCode::RawBufferLoad,
                                     CI->getCalledFunction());
      if (ValCtx.DL.getTypeAllocSizeInBits(Ty) > 32) {
        ValCtx.EmitInstrError(CI, ValidationRule::Sm64bitRawBufferLoadStore);
      }
    }
    DxilInst_RawBufferLoad bufLd(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(bufLd.get_srv(), compTy, resClass, ValCtx);

    if (resClass != DXIL::ResourceClass::SRV &&
        resClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);
    }

    Value *offset = bufLd.get_elementOffset();
    Value *align = bufLd.get_alignment();
    unsigned alignSize = 0;
    if (!isa<ConstantInt>(align)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrCoordinateCountForRawTypedBuf);
    } else {
      alignSize = bufLd.get_alignment_val();
    }
    switch (resKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrCoordinateCountForStructBuf);
      }
      break;
    default:
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrResourceKindForBufferLoadStore);
      break;
    }
  } break;
  case DXIL::OpCode::RawBufferStore: {
    if (!ValCtx.DxilMod.GetShaderModel()->IsSM63Plus()) {
      Type *Ty = OP::GetOverloadType(DXIL::OpCode::RawBufferStore,
                                     CI->getCalledFunction());
      if (ValCtx.DL.getTypeAllocSizeInBits(Ty) > 32) {
        ValCtx.EmitInstrError(CI, ValidationRule::Sm64bitRawBufferLoadStore);
      }
    }
    DxilInst_RawBufferStore bufSt(CI);
    DXIL::ComponentType compTy;
    DXIL::ResourceClass resClass;
    DXIL::ResourceKind resKind =
        GetResourceKindAndCompTy(bufSt.get_uav(), compTy, resClass, ValCtx);

    if (resClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);
    }

    ConstantInt *mask = dyn_cast<ConstantInt>(bufSt.get_mask());
    unsigned stValMask =
        StoreValueToMask({bufSt.get_value0(), bufSt.get_value1(),
                          bufSt.get_value2(), bufSt.get_value3()});

    if (!ValidateStorageMasks(CI, opcode, mask, stValMask, false /*isTyped*/,
                              ValCtx))
      return;

    Value *offset = bufSt.get_elementOffset();
    Value *align = bufSt.get_alignment();
    unsigned alignSize = 0;
    if (!isa<ConstantInt>(align)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrCoordinateCountForRawTypedBuf);
    } else {
      alignSize = bufSt.get_alignment_val();
    }
    switch (resKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(offset)) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrCoordinateCountForStructBuf);
      }
      break;
    default:
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrResourceKindForBufferLoadStore);
      break;
    }
  } break;
  case DXIL::OpCode::TraceRay: {
    DxilInst_TraceRay traceRay(CI);
    Value *hdl = traceRay.get_AccelerationStructure();
    DxilResourceProperties RP = ValCtx.GetResourceFromVal(hdl);
    if (RP.getResourceClass() == DXIL::ResourceClass::Invalid) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForTraceRay);
      return;
    }
    if (RP.getResourceKind() != DXIL::ResourceKind::RTAccelerationStructure) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForTraceRay);
    }
  } break;
  default:
    break;
  }
}

static void ValidateBarrierFlagArg(ValidationContext &ValCtx, CallInst *CI,
                                   Value *Arg, unsigned validMask,
                                   StringRef flagName, StringRef opName) {
  if (ConstantInt *CArg = dyn_cast<ConstantInt>(Arg)) {
    if ((CArg->getLimitedValue() & (uint32_t)(~validMask)) != 0) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrBarrierFlagInvalid,
                                  {flagName, opName});
    }
  } else {
    ValCtx.EmitInstrError(CI,
                          ValidationRule::InstrBarrierNonConstantFlagArgument);
  }
}

std::string GetLaunchTypeStr(DXIL::NodeLaunchType LT) {
  switch (LT) {
  case DXIL::NodeLaunchType::Broadcasting:
    return "Broadcasting";
  case DXIL::NodeLaunchType::Coalescing:
    return "Coalescing";
  case DXIL::NodeLaunchType::Thread:
    return "Thread";
  default:
    return "Invalid";
  }
}

static void ValidateDxilOperationCallInProfile(CallInst *CI,
                                               DXIL::OpCode opcode,
                                               const ShaderModel *pSM,
                                               ValidationContext &ValCtx) {
  DXIL::ShaderKind shaderKind =
      pSM ? pSM->GetKind() : DXIL::ShaderKind::Invalid;
  llvm::Function *F = CI->getParent()->getParent();
  DXIL::NodeLaunchType nodeLaunchType = DXIL::NodeLaunchType::Invalid;
  if (DXIL::ShaderKind::Library == shaderKind) {
    if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
      DxilEntryProps &entryProps = ValCtx.DxilMod.GetDxilEntryProps(F);
      shaderKind = ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
      if (shaderKind == DXIL::ShaderKind::Node)
        nodeLaunchType = entryProps.props.Node.LaunchType;

    } else if (ValCtx.DxilMod.IsPatchConstantShader(F))
      shaderKind = DXIL::ShaderKind::Hull;
  }

  // These shader models are treted like compute
  bool isCSLike = shaderKind == DXIL::ShaderKind::Compute ||
                  shaderKind == DXIL::ShaderKind::Mesh ||
                  shaderKind == DXIL::ShaderKind::Amplification ||
                  shaderKind == DXIL::ShaderKind::Node;
  // Is called from a library function
  bool isLibFunc = shaderKind == DXIL::ShaderKind::Library;

  ValidateHandleArgs(CI, opcode, ValCtx);

  switch (opcode) {
  // Imm input value validation.
  case DXIL::OpCode::Asin:
  case DXIL::OpCode::Acos:
  case DXIL::OpCode::Log:
  case DXIL::OpCode::DerivFineX:
  case DXIL::OpCode::DerivFineY:
  case DXIL::OpCode::DerivCoarseX:
  case DXIL::OpCode::DerivCoarseY:
    ValidateImmOperandForMathDxilOp(CI, opcode, ValCtx);
    break;
  // Resource validation.
  case DXIL::OpCode::GetDimensions:
  case DXIL::OpCode::CalculateLOD:
  case DXIL::OpCode::TextureGather:
  case DXIL::OpCode::TextureGatherCmp:
  case DXIL::OpCode::Sample:
  case DXIL::OpCode::SampleCmp:
  case DXIL::OpCode::SampleCmpLevel:
  case DXIL::OpCode::SampleCmpLevelZero:
  case DXIL::OpCode::SampleBias:
  case DXIL::OpCode::SampleGrad:
  case DXIL::OpCode::SampleCmpBias:
  case DXIL::OpCode::SampleCmpGrad:
  case DXIL::OpCode::SampleLevel:
  case DXIL::OpCode::CheckAccessFullyMapped:
  case DXIL::OpCode::BufferStore:
  case DXIL::OpCode::TextureStore:
  case DXIL::OpCode::BufferLoad:
  case DXIL::OpCode::TextureLoad:
  case DXIL::OpCode::CBufferLoad:
  case DXIL::OpCode::CBufferLoadLegacy:
  case DXIL::OpCode::RawBufferLoad:
  case DXIL::OpCode::RawBufferStore:
    ValidateResourceDxilOp(CI, opcode, ValCtx);
    break;
  // Input output.
  case DXIL::OpCode::LoadInput:
  case DXIL::OpCode::DomainLocation:
  case DXIL::OpCode::StoreOutput:
  case DXIL::OpCode::StoreVertexOutput:
  case DXIL::OpCode::StorePrimitiveOutput:
  case DXIL::OpCode::OutputControlPointID:
  case DXIL::OpCode::LoadOutputControlPoint:
  case DXIL::OpCode::StorePatchConstant:
  case DXIL::OpCode::Coverage:
  case DXIL::OpCode::InnerCoverage:
  case DXIL::OpCode::ViewID:
  case DXIL::OpCode::EvalCentroid:
  case DXIL::OpCode::EvalSampleIndex:
  case DXIL::OpCode::EvalSnapped:
  case DXIL::OpCode::AttributeAtVertex:
  case DXIL::OpCode::EmitStream:
  case DXIL::OpCode::EmitThenCutStream:
  case DXIL::OpCode::CutStream:
    ValidateSignatureDxilOp(CI, opcode, ValCtx);
    break;
  // Special.
  case DXIL::OpCode::AllocateRayQuery: {
    // validate flags are immediate and compatible
    llvm::Value *constRayFlag = CI->getOperand(1);
    if (!llvm::isa<llvm::Constant>(constRayFlag)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::DeclAllocateRayQueryFlagsAreConst);
    }
    break;
  }
  case DXIL::OpCode::AllocateRayQuery2: {
    // validate flags are immediate and compatible
    llvm::Value *constRayFlag = CI->getOperand(1);
    llvm::Value *RayQueryFlag = CI->getOperand(2);
    if (!llvm::isa<llvm::Constant>(constRayFlag) ||
        !llvm::isa<llvm::Constant>(RayQueryFlag)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::DeclAllocateRayQuery2FlagsAreConst);
      break;
    }
    // When the ForceOMM2State ConstRayFlag is given as an argument to
    // a RayQuery object, AllowOpacityMicromaps is expected
    // as a RayQueryFlag argument
    llvm::ConstantInt *Arg1 = llvm::cast<llvm::ConstantInt>(constRayFlag);
    llvm::ConstantInt *Arg2 = llvm::cast<llvm::ConstantInt>(RayQueryFlag);
    if ((Arg1->getValue().getSExtValue() &
         (unsigned)DXIL::RayFlag::ForceOMM2State) &&
        (Arg2->getValue().getSExtValue() &
         (unsigned)DXIL::RayQueryFlag::AllowOpacityMicromaps) == 0) {
      ValCtx.EmitInstrError(
          CI,
          ValidationRule::DeclAllowOpacityMicromapsExpectedGivenForceOMM2State);
    }
    break;
  }

  case DXIL::OpCode::BufferUpdateCounter: {
    DxilInst_BufferUpdateCounter updateCounter(CI);
    Value *handle = updateCounter.get_uav();
    DxilResourceProperties RP = ValCtx.GetResourceFromVal(handle);

    if (!RP.isUAV()) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBufferUpdateCounterOnUAV);
    }

    if (!DXIL::IsStructuredBuffer(RP.getResourceKind())) {
      ValCtx.EmitInstrError(CI, ValidationRule::SmCounterOnlyOnStructBuf);
    }

    if (!RP.Basic.SamplerCmpOrHasCounter) {
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrBufferUpdateCounterOnResHasCounter);
    }

    Value *inc = updateCounter.get_inc();
    if (ConstantInt *cInc = dyn_cast<ConstantInt>(inc)) {
      bool isInc = cInc->getLimitedValue() == 1;
      if (!ValCtx.isLibProfile) {
        auto it = ValCtx.HandleResIndexMap.find(handle);
        if (it != ValCtx.HandleResIndexMap.end()) {
          unsigned resIndex = it->second;
          if (ValCtx.UavCounterIncMap.count(resIndex)) {
            if (isInc != ValCtx.UavCounterIncMap[resIndex]) {
              ValCtx.EmitInstrError(CI,
                                    ValidationRule::InstrOnlyOneAllocConsume);
            }
          } else {
            ValCtx.UavCounterIncMap[resIndex] = isInc;
          }
        }

      } else {
        // TODO: validate ValidationRule::InstrOnlyOneAllocConsume for lib
        // profile.
      }
    } else {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                  {"inc", "BufferUpdateCounter"});
    }

  } break;
  case DXIL::OpCode::Barrier: {
    DxilInst_Barrier barrier(CI);
    Value *mode = barrier.get_barrierMode();
    ConstantInt *cMode = dyn_cast<ConstantInt>(mode);
    if (!cMode) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                  {"Mode", "Barrier"});
      return;
    }

    const unsigned uglobal =
        static_cast<unsigned>(DXIL::BarrierMode::UAVFenceGlobal);
    const unsigned g = static_cast<unsigned>(DXIL::BarrierMode::TGSMFence);
    const unsigned ut =
        static_cast<unsigned>(DXIL::BarrierMode::UAVFenceThreadGroup);
    unsigned barrierMode = cMode->getLimitedValue();

    if (isCSLike || isLibFunc) {
      bool bHasUGlobal = barrierMode & uglobal;
      bool bHasGroup = barrierMode & g;
      bool bHasUGroup = barrierMode & ut;
      if (bHasUGlobal && bHasUGroup) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrBarrierModeUselessUGroup);
      }
      if (!bHasUGlobal && !bHasGroup && !bHasUGroup) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeNoMemory);
      }
    } else {
      if (uglobal != barrierMode) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeForNonCS);
      }
    }

  } break;
  case DXIL::OpCode::BarrierByMemoryType: {
    DxilInst_BarrierByMemoryType DI(CI);
    ValidateBarrierFlagArg(ValCtx, CI, DI.get_MemoryTypeFlags(),
                           (unsigned)hlsl::DXIL::MemoryTypeFlag::ValidMask,
                           "memory type", "BarrierByMemoryType");
    ValidateBarrierFlagArg(ValCtx, CI, DI.get_SemanticFlags(),
                           (unsigned)hlsl::DXIL::BarrierSemanticFlag::ValidMask,
                           "semantic", "BarrierByMemoryType");
    if (!isLibFunc && shaderKind != DXIL::ShaderKind::Node &&
        OP::BarrierRequiresNode(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierRequiresNode);
    }
    if (!isCSLike && !isLibFunc && OP::BarrierRequiresGroup(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeForNonCS);
    }
  } break;
  case DXIL::OpCode::BarrierByNodeRecordHandle:
  case DXIL::OpCode::BarrierByMemoryHandle: {
    std::string opName = opcode == DXIL::OpCode::BarrierByNodeRecordHandle
                             ? "barrierByNodeRecordHandle"
                             : "barrierByMemoryHandle";
    DxilInst_BarrierByMemoryHandle DIMH(CI);
    ValidateBarrierFlagArg(ValCtx, CI, DIMH.get_SemanticFlags(),
                           (unsigned)hlsl::DXIL::BarrierSemanticFlag::ValidMask,
                           "semantic", opName);
    if (!isLibFunc && shaderKind != DXIL::ShaderKind::Node &&
        OP::BarrierRequiresNode(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierRequiresNode);
    }
    if (!isCSLike && !isLibFunc && OP::BarrierRequiresGroup(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeForNonCS);
    }
  } break;
  case DXIL::OpCode::CreateHandleForLib:
    if (!ValCtx.isLibProfile) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"CreateHandleForLib", "Library"});
    }
    break;
  case DXIL::OpCode::AtomicBinOp:
  case DXIL::OpCode::AtomicCompareExchange: {
    Type *pOverloadType = OP::GetOverloadType(opcode, CI->getCalledFunction());
    if ((pOverloadType->isIntegerTy(64)) && !pSM->IsSM66Plus())
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"64-bit atomic operations", "Shader Model 6.6+"});
    Value *Handle = CI->getOperand(DXIL::OperandIndex::kAtomicBinOpHandleOpIdx);
    if (!isa<CallInst>(Handle) ||
        ValCtx.GetResourceFromVal(Handle).getResourceClass() !=
            DXIL::ResourceClass::UAV)
      ValCtx.EmitInstrError(CI, ValidationRule::InstrAtomicIntrinNonUAV);
  } break;
  case DXIL::OpCode::CreateHandle:
    if (ValCtx.isLibProfile) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"CreateHandle", "non-library targets"});
    }
    // CreateHandle should not be used in SM 6.6 and above:
    if (DXIL::CompareVersions(ValCtx.m_DxilMajor, ValCtx.m_DxilMinor, 1, 5) >
        0) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"CreateHandle", "Shader model 6.5 and below"});
    }
    break;

  case DXIL::OpCode::ThreadId: // SV_DispatchThreadID
    if (shaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (nodeLaunchType == DXIL::NodeLaunchType::Broadcasting)
      break;

    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrSVConflictingLaunchMode,
        {"ThreadId", "SV_DispatchThreadID", GetLaunchTypeStr(nodeLaunchType)});
    break;

  case DXIL::OpCode::GroupId: // SV_GroupId
    if (shaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (nodeLaunchType == DXIL::NodeLaunchType::Broadcasting)
      break;

    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrSVConflictingLaunchMode,
        {"GroupId", "SV_GroupId", GetLaunchTypeStr(nodeLaunchType)});
    break;

  case DXIL::OpCode::ThreadIdInGroup: // SV_GroupThreadID
    if (shaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (nodeLaunchType == DXIL::NodeLaunchType::Broadcasting ||
        nodeLaunchType == DXIL::NodeLaunchType::Coalescing)
      break;

    ValCtx.EmitInstrFormatError(CI,
                                ValidationRule::InstrSVConflictingLaunchMode,
                                {"ThreadIdInGroup", "SV_GroupThreadID",
                                 GetLaunchTypeStr(nodeLaunchType)});

    break;

  case DXIL::OpCode::FlattenedThreadIdInGroup: // SV_GroupIndex
    if (shaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (nodeLaunchType == DXIL::NodeLaunchType::Broadcasting ||
        nodeLaunchType == DXIL::NodeLaunchType::Coalescing)
      break;

    ValCtx.EmitInstrFormatError(CI,
                                ValidationRule::InstrSVConflictingLaunchMode,
                                {"FlattenedThreadIdInGroup", "SV_GroupIndex",
                                 GetLaunchTypeStr(nodeLaunchType)});

    break;

  default:
    // TODO: make sure every opcode is checked.
    // Skip opcodes don't need special check.
    break;
  }
}

static bool IsDxilFunction(llvm::Function *F) {
  unsigned argSize = F->arg_size();
  if (argSize < 1) {
    // Cannot be a DXIL operation.
    return false;
  }

  return OP::IsDxilOpFunc(F);
}

static bool IsLifetimeIntrinsic(llvm::Function *F) {
  return (F->isIntrinsic() &&
          (F->getIntrinsicID() == Intrinsic::lifetime_start ||
           F->getIntrinsicID() == Intrinsic::lifetime_end));
}

static void ValidateExternalFunction(Function *F, ValidationContext &ValCtx) {
  if (DXIL::CompareVersions(ValCtx.m_DxilMajor, ValCtx.m_DxilMinor, 1, 6) >=
          0 &&
      IsLifetimeIntrinsic(F)) {
    // TODO: validate lifetime intrinsic users
    return;
  }

  if (!IsDxilFunction(F) && !ValCtx.isLibProfile) {
    ValCtx.EmitFnFormatError(F, ValidationRule::DeclDxilFnExtern,
                             {F->getName()});
    return;
  }

  if (F->use_empty()) {
    ValCtx.EmitFnFormatError(F, ValidationRule::DeclUsedExternalFunction,
                             {F->getName()});
    return;
  }

  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
  OP *hlslOP = ValCtx.DxilMod.GetOP();
  bool isDxilOp = OP::IsDxilOpFunc(F);
  Type *voidTy = Type::getVoidTy(F->getContext());

  for (User *user : F->users()) {
    CallInst *CI = dyn_cast<CallInst>(user);
    if (!CI) {
      ValCtx.EmitFnFormatError(F, ValidationRule::DeclFnIsCalled,
                               {F->getName()});
      continue;
    }

    // Skip call to external user defined function
    if (!isDxilOp)
      continue;

    Value *argOpcode = CI->getArgOperand(0);
    ConstantInt *constOpcode = dyn_cast<ConstantInt>(argOpcode);
    if (!constOpcode) {
      // opcode not immediate; function body will validate this error.
      continue;
    }

    unsigned opcode = constOpcode->getLimitedValue();
    if (opcode >= (unsigned)DXIL::OpCode::NumOpCodes) {
      // invalid opcode; function body will validate this error.
      continue;
    }

    DXIL::OpCode dxilOpcode = (DXIL::OpCode)opcode;

    // In some cases, no overloads are provided (void is exclusive to others)
    Function *dxilFunc;
    if (hlslOP->IsOverloadLegal(dxilOpcode, voidTy)) {
      dxilFunc = hlslOP->GetOpFunc(dxilOpcode, voidTy);
    } else {
      Type *Ty = OP::GetOverloadType(dxilOpcode, CI->getCalledFunction());
      try {
        if (!hlslOP->IsOverloadLegal(dxilOpcode, Ty)) {
          ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
          continue;
        }
      } catch (...) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
        continue;
      }
      dxilFunc = hlslOP->GetOpFunc(dxilOpcode, Ty);
    }

    if (!dxilFunc) {
      // Cannot find dxilFunction based on opcode and type.
      ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
      continue;
    }

    if (dxilFunc->getFunctionType() != F->getFunctionType()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrCallOload,
                                  {dxilFunc->getName()});
      continue;
    }

    unsigned major = pSM->GetMajor();
    unsigned minor = pSM->GetMinor();
    if (ValCtx.isLibProfile) {
      Function *callingFunction = CI->getParent()->getParent();
      DXIL::ShaderKind SK = DXIL::ShaderKind::Library;
      if (ValCtx.DxilMod.HasDxilFunctionProps(callingFunction))
        SK = ValCtx.DxilMod.GetDxilFunctionProps(callingFunction).shaderKind;
      else if (ValCtx.DxilMod.IsPatchConstantShader(callingFunction))
        SK = DXIL::ShaderKind::Hull;
      if (!ValidateOpcodeInProfile(dxilOpcode, SK, major, minor)) {
        // Opcode not available in profile.
        // produces: "lib_6_3(ps)", or "lib_6_3(anyhit)" for shader types
        // Or: "lib_6_3(lib)" for library function
        std::string shaderModel = pSM->GetName();
        shaderModel += std::string("(") + ShaderModel::GetKindName(SK) + ")";
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::SmOpcode,
            {hlslOP->GetOpCodeName(dxilOpcode), shaderModel});
        continue;
      }
    } else {
      if (!ValidateOpcodeInProfile(dxilOpcode, pSM->GetKind(), major, minor)) {
        // Opcode not available in profile.
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::SmOpcode,
            {hlslOP->GetOpCodeName(dxilOpcode), pSM->GetName()});
        continue;
      }
    }

    // Check more detail.
    ValidateDxilOperationCallInProfile(CI, dxilOpcode, pSM, ValCtx);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Instruction validation functions.                                         //

static bool IsDxilBuiltinStructType(StructType *ST, hlsl::OP *hlslOP) {
  if (ST == hlslOP->GetBinaryWithCarryType())
    return true;
  if (ST == hlslOP->GetBinaryWithTwoOutputsType())
    return true;
  if (ST == hlslOP->GetFourI32Type())
    return true;
  if (ST == hlslOP->GetFourI16Type())
    return true;
  if (ST == hlslOP->GetDimensionsType())
    return true;
  if (ST == hlslOP->GetHandleType())
    return true;
  if (ST == hlslOP->GetSamplePosType())
    return true;
  if (ST == hlslOP->GetSplitDoubleType())
    return true;

  unsigned EltNum = ST->getNumElements();
  Type *EltTy = ST->getElementType(0);
  switch (EltNum) {
  case 2:
    // Check if it's a native vector resret.
    if (EltTy->isVectorTy())
      return ST == hlslOP->GetResRetType(EltTy);
    LLVM_FALLTHROUGH;
  case 4:
  case 8: // 2 for doubles, 8 for halfs.
    return ST == hlslOP->GetCBufferRetType(EltTy);
    break;
  case 5:
    return ST == hlslOP->GetResRetType(EltTy);
    break;
  default:
    return false;
  }
}

// outer type may be: [ptr to][1 dim array of]( UDT struct | scalar )
// inner type (UDT struct member) may be: [N dim array of]( UDT struct | scalar
// ) scalar type may be: ( float(16|32|64) | int(16|32|64) )
static bool ValidateType(Type *Ty, ValidationContext &ValCtx,
                         bool bInner = false) {
  DXASSERT_NOMSG(Ty != nullptr);
  if (Ty->isPointerTy()) {
    Type *EltTy = Ty->getPointerElementType();
    if (bInner || EltTy->isPointerTy()) {
      ValCtx.EmitTypeError(Ty, ValidationRule::TypesNoPtrToPtr);
      return false;
    }
    Ty = EltTy;
  }
  if (Ty->isArrayTy()) {
    Type *EltTy = Ty->getArrayElementType();
    if (!bInner && isa<ArrayType>(EltTy)) {
      // Outermost array should be converted to single-dim,
      // but arrays inside struct are allowed to be multi-dim
      ValCtx.EmitTypeError(Ty, ValidationRule::TypesNoMultiDim);
      return false;
    }
    while (EltTy->isArrayTy())
      EltTy = EltTy->getArrayElementType();
    Ty = EltTy;
  }
  if (Ty->isStructTy()) {
    bool result = true;
    StructType *ST = cast<StructType>(Ty);

    StringRef Name = ST->getName();
    if (Name.startswith("dx.")) {
      // Allow handle type.
      if (ValCtx.HandleTy == Ty)
        return true;
      hlsl::OP *hlslOP = ValCtx.DxilMod.GetOP();
      if (IsDxilBuiltinStructType(ST, hlslOP)) {
        ValCtx.EmitTypeError(Ty, ValidationRule::InstrDxilStructUser);
        result = false;
      }

      ValCtx.EmitTypeError(Ty, ValidationRule::DeclDxilNsReserved);
      result = false;
    }
    for (auto e : ST->elements()) {
      if (!ValidateType(e, ValCtx, /*bInner*/ true)) {
        result = false;
      }
    }
    return result;
  }
  if (Ty->isFloatTy() || Ty->isHalfTy() || Ty->isDoubleTy()) {
    return true;
  }
  if (Ty->isIntegerTy()) {
    unsigned width = Ty->getIntegerBitWidth();
    if (width != 1 && width != 8 && width != 16 && width != 32 && width != 64) {
      ValCtx.EmitTypeError(Ty, ValidationRule::TypesIntWidth);
      return false;
    }
    return true;
  }
  // Lib profile allow all types except those hit
  // ValidationRule::InstrDxilStructUser.
  if (ValCtx.isLibProfile)
    return true;

  if (Ty->isVectorTy()) {
    if (Ty->getVectorNumElements() > 1 &&
        ValCtx.DxilMod.GetShaderModel()->IsSM69Plus())
      return true;
    ValCtx.EmitTypeError(Ty, ValidationRule::TypesNoVector);
    return false;
  }
  ValCtx.EmitTypeError(Ty, ValidationRule::TypesDefined);
  return false;
}

static bool GetNodeOperandAsInt(ValidationContext &ValCtx, MDNode *pMD,
                                unsigned index, uint64_t *pValue) {
  *pValue = 0;
  if (pMD->getNumOperands() < index) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  ConstantAsMetadata *C = dyn_cast<ConstantAsMetadata>(pMD->getOperand(index));
  if (C == nullptr) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  ConstantInt *CI = dyn_cast<ConstantInt>(C->getValue());
  if (CI == nullptr) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  *pValue = CI->getValue().getZExtValue();
  return true;
}

static bool IsPrecise(Instruction &I, ValidationContext &ValCtx) {
  MDNode *pMD = I.getMetadata(DxilMDHelper::kDxilPreciseAttributeMDName);
  if (pMD == nullptr) {
    return false;
  }
  if (pMD->getNumOperands() != 1) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }

  uint64_t val;
  if (!GetNodeOperandAsInt(ValCtx, pMD, 0, &val)) {
    return false;
  }
  if (val == 1) {
    return true;
  }
  if (val != 0) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaValueRange);
  }
  return false;
}

static bool IsValueMinPrec(DxilModule &DxilMod, Value *V) {
  DXASSERT(DxilMod.GetGlobalFlags() & DXIL::kEnableMinPrecision,
           "else caller didn't check - currently this path should never be hit "
           "otherwise");
  (void)(DxilMod);
  Type *Ty = V->getType();
  if (Ty->isIntegerTy()) {
    return 16 == Ty->getIntegerBitWidth();
  }
  return Ty->isHalfTy();
}

static void ValidateMsIntrinsics(Function *F, ValidationContext &ValCtx,
                                 CallInst *setMeshOutputCounts,
                                 CallInst *getMeshPayload) {
  if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
    DXIL::ShaderKind shaderKind =
        ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
    if (shaderKind != DXIL::ShaderKind::Mesh)
      return;
  } else {
    return;
  }

  DominatorTreeAnalysis DTA;
  DominatorTree DT = DTA.run(*F);

  for (auto b = F->begin(), bend = F->end(); b != bend; ++b) {
    bool foundSetMeshOutputCountsInCurrentBB = false;
    for (auto i = b->begin(), iend = b->end(); i != iend; ++i) {
      llvm::Instruction &I = *i;

      // Calls to external functions.
      CallInst *CI = dyn_cast<CallInst>(&I);
      if (CI) {
        Function *FCalled = CI->getCalledFunction();
        if (!FCalled) {
          ValCtx.EmitInstrError(&I, ValidationRule::InstrAllowed);
          continue;
        }
        if (FCalled->isDeclaration()) {
          // External function validation will diagnose.
          if (!IsDxilFunction(FCalled)) {
            continue;
          }

          if (CI == setMeshOutputCounts) {
            foundSetMeshOutputCountsInCurrentBB = true;
          }
          Value *opcodeVal = CI->getOperand(0);
          ConstantInt *OpcodeConst = dyn_cast<ConstantInt>(opcodeVal);
          unsigned opcode = OpcodeConst->getLimitedValue();
          DXIL::OpCode dxilOpcode = (DXIL::OpCode)opcode;

          if (dxilOpcode == DXIL::OpCode::StoreVertexOutput ||
              dxilOpcode == DXIL::OpCode::StorePrimitiveOutput ||
              dxilOpcode == DXIL::OpCode::EmitIndices) {
            if (setMeshOutputCounts == nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMissingSetMeshOutputCounts);
            } else if (!foundSetMeshOutputCountsInCurrentBB &&
                       !DT.dominates(setMeshOutputCounts->getParent(),
                                     I.getParent())) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrNonDominatingSetMeshOutputCounts);
            }
          }
        }
      }
    }
  }

  if (getMeshPayload) {
    PointerType *payloadPTy = cast<PointerType>(getMeshPayload->getType());
    StructType *payloadTy =
        cast<StructType>(payloadPTy->getPointerElementType());
    const DataLayout &DL = F->getParent()->getDataLayout();
    unsigned payloadSize = DL.getTypeAllocSize(payloadTy);

    DxilFunctionProps &prop = ValCtx.DxilMod.GetDxilFunctionProps(F);

    if (prop.ShaderProps.MS.payloadSizeInBytes < payloadSize) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderPayloadSizeDeclared,
          {F->getName(), std::to_string(payloadSize),
           std::to_string(prop.ShaderProps.MS.payloadSizeInBytes)});
    }

    if (prop.ShaderProps.MS.payloadSizeInBytes > DXIL::kMaxMSASPayloadBytes) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderPayloadSize,
          {F->getName(), std::to_string(prop.ShaderProps.MS.payloadSizeInBytes),
           std::to_string(DXIL::kMaxMSASPayloadBytes)});
    }
  }
}

static void ValidateAsIntrinsics(Function *F, ValidationContext &ValCtx,
                                 CallInst *dispatchMesh) {
  if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
    DXIL::ShaderKind shaderKind =
        ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
    if (shaderKind != DXIL::ShaderKind::Amplification)
      return;

    if (dispatchMesh) {
      DxilInst_DispatchMesh dispatchMeshCall(dispatchMesh);
      Value *operandVal = dispatchMeshCall.get_payload();
      Type *payloadTy = operandVal->getType();
      const DataLayout &DL = F->getParent()->getDataLayout();
      unsigned payloadSize = DL.getTypeAllocSize(payloadTy);

      DxilFunctionProps &prop = ValCtx.DxilMod.GetDxilFunctionProps(F);

      if (prop.ShaderProps.AS.payloadSizeInBytes < payloadSize) {
        ValCtx.EmitInstrFormatError(
            dispatchMesh,
            ValidationRule::SmAmplificationShaderPayloadSizeDeclared,
            {F->getName(), std::to_string(payloadSize),
             std::to_string(prop.ShaderProps.AS.payloadSizeInBytes)});
      }

      if (prop.ShaderProps.AS.payloadSizeInBytes > DXIL::kMaxMSASPayloadBytes) {
        ValCtx.EmitInstrFormatError(
            dispatchMesh, ValidationRule::SmAmplificationShaderPayloadSize,
            {F->getName(),
             std::to_string(prop.ShaderProps.AS.payloadSizeInBytes),
             std::to_string(DXIL::kMaxMSASPayloadBytes)});
      }
    }

  } else {
    return;
  }

  if (dispatchMesh == nullptr) {
    ValCtx.EmitFnError(F, ValidationRule::InstrNotOnceDispatchMesh);
    return;
  }

  PostDominatorTree PDT;
  PDT.runOnFunction(*F);

  if (!PDT.dominates(dispatchMesh->getParent(), &F->getEntryBlock())) {
    ValCtx.EmitInstrError(dispatchMesh,
                          ValidationRule::InstrNonDominatingDispatchMesh);
  }

  Function *dispatchMeshFunc = dispatchMesh->getCalledFunction();
  FunctionType *dispatchMeshFuncTy = dispatchMeshFunc->getFunctionType();
  PointerType *payloadPTy =
      cast<PointerType>(dispatchMeshFuncTy->getParamType(4));
  StructType *payloadTy = cast<StructType>(payloadPTy->getPointerElementType());
  const DataLayout &DL = F->getParent()->getDataLayout();
  unsigned payloadSize = DL.getTypeAllocSize(payloadTy);

  if (payloadSize > DXIL::kMaxMSASPayloadBytes) {
    ValCtx.EmitInstrFormatError(
        dispatchMesh, ValidationRule::SmAmplificationShaderPayloadSize,
        {F->getName(), std::to_string(payloadSize),
         std::to_string(DXIL::kMaxMSASPayloadBytes)});
  }
}

static void ValidateControlFlowHint(BasicBlock &bb, ValidationContext &ValCtx) {
  // Validate controlflow hint.
  TerminatorInst *TI = bb.getTerminator();
  if (!TI)
    return;

  MDNode *pNode = TI->getMetadata(DxilMDHelper::kDxilControlFlowHintMDName);
  if (!pNode)
    return;

  if (pNode->getNumOperands() < 3)
    return;

  bool bHasBranch = false;
  bool bHasFlatten = false;
  bool bForceCase = false;

  for (unsigned i = 2; i < pNode->getNumOperands(); i++) {
    uint64_t value = 0;
    if (GetNodeOperandAsInt(ValCtx, pNode, i, &value)) {
      DXIL::ControlFlowHint hint = static_cast<DXIL::ControlFlowHint>(value);
      switch (hint) {
      case DXIL::ControlFlowHint::Flatten:
        bHasFlatten = true;
        break;
      case DXIL::ControlFlowHint::Branch:
        bHasBranch = true;
        break;
      case DXIL::ControlFlowHint::ForceCase:
        bForceCase = true;
        break;
      default:
        ValCtx.EmitMetaError(pNode, ValidationRule::MetaInvalidControlFlowHint);
      }
    }
  }
  if (bHasBranch && bHasFlatten) {
    ValCtx.EmitMetaError(pNode, ValidationRule::MetaBranchFlatten);
  }
  if (bForceCase && !isa<SwitchInst>(TI)) {
    ValCtx.EmitMetaError(pNode, ValidationRule::MetaForceCaseOnSwitch);
  }
}

static void ValidateTBAAMetadata(MDNode *Node, ValidationContext &ValCtx) {
  switch (Node->getNumOperands()) {
  case 1: {
    if (Node->getOperand(0)->getMetadataID() != Metadata::MDStringKind) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    }
  } break;
  case 2: {
    MDNode *rootNode = dyn_cast<MDNode>(Node->getOperand(1));
    if (!rootNode) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ValidateTBAAMetadata(rootNode, ValCtx);
    }
  } break;
  case 3: {
    MDNode *rootNode = dyn_cast<MDNode>(Node->getOperand(1));
    if (!rootNode) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ValidateTBAAMetadata(rootNode, ValCtx);
    }
    ConstantAsMetadata *pointsToConstMem =
        dyn_cast<ConstantAsMetadata>(Node->getOperand(2));
    if (!pointsToConstMem) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ConstantInt *isConst =
          dyn_cast<ConstantInt>(pointsToConstMem->getValue());
      if (!isConst) {
        ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
      } else if (isConst->getValue().getLimitedValue() > 1) {
        ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
      }
    }
  } break;
  default:
    ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
  }
}

static void ValidateLoopMetadata(MDNode *Node, ValidationContext &ValCtx) {
  if (Node->getNumOperands() == 0 || Node->getNumOperands() > 2) {
    ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    return;
  }
  if (Node != Node->getOperand(0).get()) {
    ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    return;
  }
  if (Node->getNumOperands() == 1) {
    return;
  }

  MDNode *LoopNode = dyn_cast<MDNode>(Node->getOperand(1).get());
  if (!LoopNode) {
    ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    return;
  }

  if (LoopNode->getNumOperands() < 1 || LoopNode->getNumOperands() > 2) {
    ValCtx.EmitMetaError(LoopNode, ValidationRule::MetaWellFormed);
    return;
  }

  if (LoopNode->getOperand(0) == LoopNode) {
    ValidateLoopMetadata(LoopNode, ValCtx);
    return;
  }

  MDString *LoopStr = dyn_cast<MDString>(LoopNode->getOperand(0));
  if (!LoopStr) {
    ValCtx.EmitMetaError(LoopNode, ValidationRule::MetaWellFormed);
    return;
  }

  StringRef Name = LoopStr->getString();
  if (Name != "llvm.loop.unroll.full" && Name != "llvm.loop.unroll.disable" &&
      Name != "llvm.loop.unroll.count") {
    ValCtx.EmitMetaError(LoopNode, ValidationRule::MetaWellFormed);
    return;
  }

  if (Name == "llvm.loop.unroll.count") {
    if (LoopNode->getNumOperands() != 2) {
      ValCtx.EmitMetaError(LoopNode, ValidationRule::MetaWellFormed);
      return;
    }
    ConstantAsMetadata *CountNode =
        dyn_cast<ConstantAsMetadata>(LoopNode->getOperand(1));
    if (!CountNode) {
      ValCtx.EmitMetaError(LoopNode, ValidationRule::MetaWellFormed);
    } else {
      ConstantInt *Count = dyn_cast<ConstantInt>(CountNode->getValue());
      if (!Count) {
        ValCtx.EmitMetaError(CountNode, ValidationRule::MetaWellFormed);
      }
    }
  }
}

static void ValidateNonUniformMetadata(Instruction &I, MDNode *pMD,
                                       ValidationContext &ValCtx) {
  if (!ValCtx.isLibProfile) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaUsed);
  }
  if (!isa<GetElementPtrInst>(I)) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
  }
  if (pMD->getNumOperands() != 1) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
  }
  uint64_t val;
  if (!GetNodeOperandAsInt(ValCtx, pMD, 0, &val)) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
  }
  if (val != 1) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaValueRange);
  }
}

static void ValidateInstructionMetadata(Instruction *I,
                                        ValidationContext &ValCtx) {
  SmallVector<std::pair<unsigned, MDNode *>, 2> MDNodes;
  I->getAllMetadataOtherThanDebugLoc(MDNodes);
  for (auto &MD : MDNodes) {
    if (MD.first == ValCtx.kDxilControlFlowHintMDKind) {
      if (!isa<TerminatorInst>(I)) {
        ValCtx.EmitInstrError(
            I, ValidationRule::MetaControlFlowHintNotOnControlFlow);
      }
    } else if (MD.first == ValCtx.kDxilPreciseMDKind) {
      // Validated in IsPrecise.
    } else if (MD.first == ValCtx.kLLVMLoopMDKind) {
      ValidateLoopMetadata(MD.second, ValCtx);
    } else if (MD.first == LLVMContext::MD_tbaa) {
      ValidateTBAAMetadata(MD.second, ValCtx);
    } else if (MD.first == LLVMContext::MD_range) {
      // Validated in Verifier.cpp.
    } else if (MD.first == LLVMContext::MD_noalias ||
               MD.first == LLVMContext::MD_alias_scope) {
      // noalias for DXIL validator >= 1.2
    } else if (MD.first == ValCtx.kDxilNonUniformMDKind) {
      ValidateNonUniformMetadata(*I, MD.second, ValCtx);
    } else {
      ValCtx.EmitMetaError(MD.second, ValidationRule::MetaUsed);
    }
  }
}

static void ValidateFunctionAttribute(Function *F, ValidationContext &ValCtx) {
  AttributeSet attrSet = F->getAttributes().getFnAttributes();
  // fp32-denorm-mode
  if (attrSet.hasAttribute(AttributeSet::FunctionIndex,
                           DXIL::kFP32DenormKindString)) {
    Attribute attr = attrSet.getAttribute(AttributeSet::FunctionIndex,
                                          DXIL::kFP32DenormKindString);
    StringRef value = attr.getValueAsString();
    if (!value.equals(DXIL::kFP32DenormValueAnyString) &&
        !value.equals(DXIL::kFP32DenormValueFtzString) &&
        !value.equals(DXIL::kFP32DenormValuePreserveString)) {
      ValCtx.EmitFnAttributeError(F, attr.getKindAsString(),
                                  attr.getValueAsString());
    }
  }
  // TODO: If validating libraries, we should remove all unknown function
  // attributes. For each attribute, check if it is a known attribute
  for (unsigned I = 0, E = attrSet.getNumSlots(); I != E; ++I) {
    for (auto AttrIter = attrSet.begin(I), AttrEnd = attrSet.end(I);
         AttrIter != AttrEnd; ++AttrIter) {
      if (!AttrIter->isStringAttribute()) {
        continue;
      }
      StringRef kind = AttrIter->getKindAsString();
      if (!kind.equals(DXIL::kFP32DenormKindString) &&
          !kind.equals(DXIL::kWaveOpsIncludeHelperLanesString)) {
        ValCtx.EmitFnAttributeError(F, AttrIter->getKindAsString(),
                                    AttrIter->getValueAsString());
      }
    }
  }
}

static void ValidateFunctionMetadata(Function *F, ValidationContext &ValCtx) {
  SmallVector<std::pair<unsigned, MDNode *>, 2> MDNodes;
  F->getAllMetadata(MDNodes);
  for (auto &MD : MDNodes) {
    ValCtx.EmitMetaError(MD.second, ValidationRule::MetaUsed);
  }
}

static bool IsLLVMInstructionAllowedForLib(Instruction &I,
                                           ValidationContext &ValCtx) {
  if (!(ValCtx.isLibProfile || ValCtx.DxilMod.GetShaderModel()->IsMS() ||
        ValCtx.DxilMod.GetShaderModel()->IsAS()))
    return false;
  switch (I.getOpcode()) {
  case Instruction::InsertElement:
  case Instruction::ExtractElement:
  case Instruction::ShuffleVector:
    return true;
  case Instruction::Unreachable:
    if (Instruction *Prev = I.getPrevNode()) {
      if (CallInst *CI = dyn_cast<CallInst>(Prev)) {
        Function *F = CI->getCalledFunction();
        if (IsDxilFunction(F) &&
            F->hasFnAttribute(Attribute::AttrKind::NoReturn)) {
          return true;
        }
      }
    }
    return false;
  default:
    return false;
  }
}

// Shader model specific checks for valid LLVM instructions.
// Currently only checks for pre 6.9 usage of vector operations.
// Returns false if shader model is pre 6.9 and I represents a vector
// operation. Returns true otherwise.
static bool IsLLVMInstructionAllowedForShaderModel(Instruction &I,
                                                   ValidationContext &ValCtx) {
  if (ValCtx.DxilMod.GetShaderModel()->IsSM69Plus())
    return true;
  unsigned OpCode = I.getOpcode();
  if (OpCode == Instruction::InsertElement ||
      OpCode == Instruction::ExtractElement ||
      OpCode == Instruction::ShuffleVector)
    return false;

  return true;
}

static void ValidateFunctionBody(Function *F, ValidationContext &ValCtx) {
  bool SupportsMinPrecision =
      ValCtx.DxilMod.GetGlobalFlags() & DXIL::kEnableMinPrecision;
  bool SupportsLifetimeIntrinsics =
      ValCtx.DxilMod.GetShaderModel()->IsSM66Plus();
  SmallVector<CallInst *, 16> gradientOps;
  SmallVector<CallInst *, 16> barriers;
  CallInst *setMeshOutputCounts = nullptr;
  CallInst *getMeshPayload = nullptr;
  CallInst *dispatchMesh = nullptr;
  hlsl::OP *hlslOP = ValCtx.DxilMod.GetOP();

  for (auto b = F->begin(), bend = F->end(); b != bend; ++b) {
    for (auto i = b->begin(), iend = b->end(); i != iend; ++i) {
      llvm::Instruction &I = *i;

      if (I.hasMetadata()) {

        ValidateInstructionMetadata(&I, ValCtx);
      }

      // Instructions must be allowed.
      if (!IsLLVMInstructionAllowed(I) ||
          !IsLLVMInstructionAllowedForShaderModel(I, ValCtx)) {
        if (!IsLLVMInstructionAllowedForLib(I, ValCtx)) {
          ValCtx.EmitInstrError(&I, ValidationRule::InstrAllowed);
          continue;
        }
      }

      // Instructions marked precise may not have minprecision arguments.
      if (SupportsMinPrecision) {
        if (IsPrecise(I, ValCtx)) {
          for (auto &O : I.operands()) {
            if (IsValueMinPrec(ValCtx.DxilMod, O)) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMinPrecisionNotPrecise);
              break;
            }
          }
        }
      }

      // Calls to external functions.
      CallInst *CI = dyn_cast<CallInst>(&I);
      if (CI) {
        Function *FCalled = CI->getCalledFunction();
        if (FCalled->isDeclaration()) {
          // External function validation will diagnose.
          if (!IsDxilFunction(FCalled)) {
            continue;
          }

          Value *opcodeVal = CI->getOperand(0);
          ConstantInt *OpcodeConst = dyn_cast<ConstantInt>(opcodeVal);
          if (OpcodeConst == nullptr) {
            ValCtx.EmitInstrFormatError(&I, ValidationRule::InstrOpConst,
                                        {"Opcode", "DXIL operation"});
            continue;
          }

          unsigned opcode = OpcodeConst->getLimitedValue();
          if (opcode >= static_cast<unsigned>(DXIL::OpCode::NumOpCodes)) {
            ValCtx.EmitInstrFormatError(
                &I, ValidationRule::InstrIllegalDXILOpCode,
                {std::to_string((unsigned)DXIL::OpCode::NumOpCodes),
                 std::to_string(opcode)});
            continue;
          }
          DXIL::OpCode dxilOpcode = (DXIL::OpCode)opcode;

          bool IllegalOpFunc = true;
          for (auto &it : hlslOP->GetOpFuncList(dxilOpcode)) {
            if (it.second == FCalled) {
              IllegalOpFunc = false;
              break;
            }
          }

          if (IllegalOpFunc) {
            ValCtx.EmitInstrFormatError(
                &I, ValidationRule::InstrIllegalDXILOpFunction,
                {FCalled->getName(), OP::GetOpCodeName(dxilOpcode)});
            continue;
          }

          if (OP::IsDxilOpGradient(dxilOpcode)) {
            gradientOps.push_back(CI);
          }

          if (dxilOpcode == DXIL::OpCode::Barrier) {
            barriers.push_back(CI);
          }
          // External function validation will check the parameter
          // list. This function will check that the call does not
          // violate any rules.

          if (dxilOpcode == DXIL::OpCode::SetMeshOutputCounts) {
            // validate the call count of SetMeshOutputCounts
            if (setMeshOutputCounts != nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMultipleSetMeshOutputCounts);
            }
            setMeshOutputCounts = CI;
          }

          if (dxilOpcode == DXIL::OpCode::GetMeshPayload) {
            // validate the call count of GetMeshPayload
            if (getMeshPayload != nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMultipleGetMeshPayload);
            }
            getMeshPayload = CI;
          }

          if (dxilOpcode == DXIL::OpCode::DispatchMesh) {
            // validate the call count of DispatchMesh
            if (dispatchMesh != nullptr) {
              ValCtx.EmitInstrError(&I,
                                    ValidationRule::InstrNotOnceDispatchMesh);
            }
            dispatchMesh = CI;
          }
        }
        continue;
      }

      for (Value *op : I.operands()) {
        if (isa<UndefValue>(op)) {
          bool legalUndef = isa<PHINode>(&I);
          if (isa<InsertElementInst>(&I)) {
            legalUndef = op == I.getOperand(0);
          }
          if (isa<ShuffleVectorInst>(&I)) {
            legalUndef = op == I.getOperand(1);
          }
          if (isa<StoreInst>(&I)) {
            legalUndef = op == I.getOperand(0);
          }

          if (!legalUndef)
            ValCtx.EmitInstrError(&I,
                                  ValidationRule::InstrNoReadingUninitialized);
        } else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(op)) {
          for (Value *opCE : CE->operands()) {
            if (isa<UndefValue>(opCE)) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrNoReadingUninitialized);
            }
          }
        }
        if (IntegerType *IT = dyn_cast<IntegerType>(op->getType())) {
          if (IT->getBitWidth() == 8) {
            // We always fail if we see i8 as operand type of a non-lifetime
            // instruction.
            ValCtx.EmitInstrError(&I, ValidationRule::TypesI8);
          }
        }
      }

      Type *Ty = I.getType();
      if (isa<PointerType>(Ty))
        Ty = Ty->getPointerElementType();
      while (isa<ArrayType>(Ty))
        Ty = Ty->getArrayElementType();
      if (IntegerType *IT = dyn_cast<IntegerType>(Ty)) {
        if (IT->getBitWidth() == 8) {
          // Allow i8* cast for llvm.lifetime.* intrinsics.
          if (!SupportsLifetimeIntrinsics || !isa<BitCastInst>(I) ||
              !onlyUsedByLifetimeMarkers(&I)) {
            ValCtx.EmitInstrError(&I, ValidationRule::TypesI8);
          }
        }
      }

      unsigned opcode = I.getOpcode();
      switch (opcode) {
      case Instruction::Alloca: {
        AllocaInst *AI = cast<AllocaInst>(&I);
        // TODO: validate address space and alignment
        Type *Ty = AI->getAllocatedType();
        if (!ValidateType(Ty, ValCtx)) {
          continue;
        }
      } break;
      case Instruction::ExtractValue: {
        ExtractValueInst *EV = cast<ExtractValueInst>(&I);
        Type *Ty = EV->getAggregateOperand()->getType();
        if (StructType *ST = dyn_cast<StructType>(Ty)) {
          Value *Agg = EV->getAggregateOperand();
          if (!isa<AtomicCmpXchgInst>(Agg) &&
              !IsDxilBuiltinStructType(ST, ValCtx.DxilMod.GetOP())) {
            ValCtx.EmitInstrError(EV, ValidationRule::InstrExtractValue);
          }
        } else {
          ValCtx.EmitInstrError(EV, ValidationRule::InstrExtractValue);
        }
      } break;
      case Instruction::Load: {
        Type *Ty = I.getType();
        if (!ValidateType(Ty, ValCtx)) {
          continue;
        }
      } break;
      case Instruction::Store: {
        StoreInst *SI = cast<StoreInst>(&I);
        Type *Ty = SI->getValueOperand()->getType();
        if (!ValidateType(Ty, ValCtx)) {
          continue;
        }
      } break;
      case Instruction::GetElementPtr: {
        Type *Ty = I.getType()->getPointerElementType();
        if (!ValidateType(Ty, ValCtx)) {
          continue;
        }
        GetElementPtrInst *GEP = cast<GetElementPtrInst>(&I);
        bool allImmIndex = true;
        for (auto Idx = GEP->idx_begin(), E = GEP->idx_end(); Idx != E; Idx++) {
          if (!isa<ConstantInt>(Idx)) {
            allImmIndex = false;
            break;
          }
        }
        if (allImmIndex) {
          const DataLayout &DL = ValCtx.DL;

          Value *Ptr = GEP->getPointerOperand();
          unsigned size =
              DL.getTypeAllocSize(Ptr->getType()->getPointerElementType());
          unsigned valSize =
              DL.getTypeAllocSize(GEP->getType()->getPointerElementType());

          SmallVector<Value *, 8> Indices(GEP->idx_begin(), GEP->idx_end());
          unsigned offset =
              DL.getIndexedOffset(GEP->getPointerOperandType(), Indices);
          if ((offset + valSize) > size) {
            ValCtx.EmitInstrError(GEP, ValidationRule::InstrInBoundsAccess);
          }
        }
      } break;
      case Instruction::SDiv: {
        BinaryOperator *BO = cast<BinaryOperator>(&I);
        Value *V = BO->getOperand(1);
        if (ConstantInt *imm = dyn_cast<ConstantInt>(V)) {
          if (imm->getValue().getLimitedValue() == 0) {
            ValCtx.EmitInstrError(BO, ValidationRule::InstrNoIDivByZero);
          }
        }
      } break;
      case Instruction::UDiv: {
        BinaryOperator *BO = cast<BinaryOperator>(&I);
        Value *V = BO->getOperand(1);
        if (ConstantInt *imm = dyn_cast<ConstantInt>(V)) {
          if (imm->getValue().getLimitedValue() == 0) {
            ValCtx.EmitInstrError(BO, ValidationRule::InstrNoUDivByZero);
          }
        }
      } break;
      case Instruction::AddrSpaceCast: {
        AddrSpaceCastInst *Cast = cast<AddrSpaceCastInst>(&I);
        unsigned ToAddrSpace = Cast->getType()->getPointerAddressSpace();
        unsigned FromAddrSpace =
            Cast->getOperand(0)->getType()->getPointerAddressSpace();
        if (ToAddrSpace != DXIL::kGenericPointerAddrSpace &&
            FromAddrSpace != DXIL::kGenericPointerAddrSpace) {
          ValCtx.EmitInstrError(Cast,
                                ValidationRule::InstrNoGenericPtrAddrSpaceCast);
        }
      } break;
      case Instruction::BitCast: {
        BitCastInst *Cast = cast<BitCastInst>(&I);
        Type *FromTy = Cast->getOperand(0)->getType();
        Type *ToTy = Cast->getType();
        // Allow i8* cast for llvm.lifetime.* intrinsics.
        if (SupportsLifetimeIntrinsics &&
            ToTy == Type::getInt8PtrTy(ToTy->getContext()))
          continue;
        if (isa<PointerType>(FromTy)) {
          FromTy = FromTy->getPointerElementType();
          ToTy = ToTy->getPointerElementType();
          unsigned FromSize = ValCtx.DL.getTypeAllocSize(FromTy);
          unsigned ToSize = ValCtx.DL.getTypeAllocSize(ToTy);
          if (FromSize != ToSize) {
            ValCtx.EmitInstrError(Cast, ValidationRule::InstrPtrBitCast);
            continue;
          }
          while (isa<ArrayType>(FromTy)) {
            FromTy = FromTy->getArrayElementType();
          }
          while (isa<ArrayType>(ToTy)) {
            ToTy = ToTy->getArrayElementType();
          }
        }
        if ((isa<StructType>(FromTy) || isa<StructType>(ToTy)) &&
            !ValCtx.isLibProfile) {
          ValCtx.EmitInstrError(Cast, ValidationRule::InstrStructBitCast);
          continue;
        }

        bool IsMinPrecisionTy = (ValCtx.DL.getTypeStoreSize(FromTy) < 4 ||
                                 ValCtx.DL.getTypeStoreSize(ToTy) < 4) &&
                                ValCtx.DxilMod.GetUseMinPrecision();
        if (IsMinPrecisionTy) {
          ValCtx.EmitInstrError(Cast, ValidationRule::InstrMinPrecisonBitCast);
        }
      } break;
      case Instruction::AtomicCmpXchg:
      case Instruction::AtomicRMW: {
        Value *Ptr = I.getOperand(AtomicRMWInst::getPointerOperandIndex());
        PointerType *ptrType = cast<PointerType>(Ptr->getType());
        Type *elType = ptrType->getElementType();
        const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
        if ((elType->isIntegerTy(64)) && !pSM->IsSM66Plus())
          ValCtx.EmitInstrFormatError(
              &I, ValidationRule::SmOpcodeInInvalidFunction,
              {"64-bit atomic operations", "Shader Model 6.6+"});

        if (ptrType->getAddressSpace() != DXIL::kTGSMAddrSpace &&
            ptrType->getAddressSpace() != DXIL::kNodeRecordAddrSpace)
          ValCtx.EmitInstrError(
              &I, ValidationRule::InstrAtomicOpNonGroupsharedOrRecord);

        // Drill through GEP and bitcasts
        while (true) {
          if (GEPOperator *GEP = dyn_cast<GEPOperator>(Ptr)) {
            Ptr = GEP->getPointerOperand();
            continue;
          }
          if (BitCastInst *BC = dyn_cast<BitCastInst>(Ptr)) {
            Ptr = BC->getOperand(0);
            continue;
          }
          break;
        }

        if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Ptr)) {
          if (GV->isConstant())
            ValCtx.EmitInstrError(&I, ValidationRule::InstrAtomicConst);
        }
      } break;
      }

      if (PointerType *PT = dyn_cast<PointerType>(I.getType())) {
        if (PT->getAddressSpace() == DXIL::kTGSMAddrSpace) {
          if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I)) {
            Value *Ptr = GEP->getPointerOperand();
            // Allow inner constant GEP
            if (isa<ConstantExpr>(Ptr) && isa<GEPOperator>(Ptr))
              Ptr = cast<GEPOperator>(Ptr)->getPointerOperand();
            if (!isa<GlobalVariable>(Ptr)) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrFailToResloveTGSMPointer);
            }
          } else if (BitCastInst *BCI = dyn_cast<BitCastInst>(&I)) {
            Value *Ptr = BCI->getOperand(0);
            // Allow inner constant GEP
            if (isa<ConstantExpr>(Ptr) && isa<GEPOperator>(Ptr))
              Ptr = cast<GEPOperator>(Ptr)->getPointerOperand();
            if (!isa<GetElementPtrInst>(Ptr) && !isa<GlobalVariable>(Ptr)) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrFailToResloveTGSMPointer);
            }
          } else {
            ValCtx.EmitInstrError(
                &I, ValidationRule::InstrFailToResloveTGSMPointer);
          }
        }
      }
    }
    ValidateControlFlowHint(*b, ValCtx);
  }

  ValidateMsIntrinsics(F, ValCtx, setMeshOutputCounts, getMeshPayload);

  ValidateAsIntrinsics(F, ValCtx, dispatchMesh);
}

static void ValidateNodeInputRecord(Function *F, ValidationContext &ValCtx) {
  // if there are no function props or LaunchType is Invalid, there is nothing
  // to do here
  if (!ValCtx.DxilMod.HasDxilFunctionProps(F))
    return;
  auto &props = ValCtx.DxilMod.GetDxilFunctionProps(F);
  if (!props.IsNode())
    return;
  if (props.InputNodes.size() > 1) {
    ValCtx.EmitFnFormatError(
        F, ValidationRule::DeclMultipleNodeInputs,
        {F->getName(), std::to_string(props.InputNodes.size())});
  }
  for (auto &input : props.InputNodes) {
    if (!input.Flags.RecordTypeMatchesLaunchType(props.Node.LaunchType)) {
      // We allow EmptyNodeInput here, as that may have been added implicitly
      // if there was no input specified
      if (input.Flags.IsEmptyInput())
        continue;

      llvm::StringRef validInputs = "";
      switch (props.Node.LaunchType) {
      case DXIL::NodeLaunchType::Broadcasting:
        validInputs = "{RW}DispatchNodeInputRecord";
        break;
      case DXIL::NodeLaunchType::Coalescing:
        validInputs = "{RW}GroupNodeInputRecords or EmptyNodeInput";
        break;
      case DXIL::NodeLaunchType::Thread:
        validInputs = "{RW}ThreadNodeInputRecord";
        break;
      default:
        llvm_unreachable("invalid launch type");
      }
      ValCtx.EmitFnFormatError(
          F, ValidationRule::DeclNodeLaunchInputType,
          {ShaderModel::GetNodeLaunchTypeName(props.Node.LaunchType),
           F->getName(), validInputs});
    }
  }
}

static void ValidateFunction(Function &F, ValidationContext &ValCtx) {
  if (F.isDeclaration()) {
    ValidateExternalFunction(&F, ValCtx);
    if (F.isIntrinsic() || IsDxilFunction(&F))
      return;
  } else {
    DXIL::ShaderKind shaderKind = DXIL::ShaderKind::Library;
    bool isShader = ValCtx.DxilMod.HasDxilFunctionProps(&F);
    unsigned numUDTShaderArgs = 0;
    if (isShader) {
      shaderKind = ValCtx.DxilMod.GetDxilFunctionProps(&F).shaderKind;
      switch (shaderKind) {
      case DXIL::ShaderKind::AnyHit:
      case DXIL::ShaderKind::ClosestHit:
        numUDTShaderArgs = 2;
        break;
      case DXIL::ShaderKind::Miss:
      case DXIL::ShaderKind::Callable:
        numUDTShaderArgs = 1;
        break;
      case DXIL::ShaderKind::Compute: {
        DxilModule &DM = ValCtx.DxilMod;
        if (DM.HasDxilEntryProps(&F)) {
          DxilEntryProps &entryProps = DM.GetDxilEntryProps(&F);
          // Check that compute has no node metadata
          if (entryProps.props.IsNode()) {
            ValCtx.EmitFnFormatError(&F, ValidationRule::MetaComputeWithNode,
                                     {F.getName()});
          }
        }
        break;
      }
      default:
        break;
      }
    } else {
      isShader = ValCtx.DxilMod.IsPatchConstantShader(&F);
    }

    // Entry function should not have parameter.
    if (isShader && 0 == numUDTShaderArgs && !F.arg_empty())
      ValCtx.EmitFnFormatError(&F, ValidationRule::FlowFunctionCall,
                               {F.getName()});

    // Shader functions should return void.
    if (isShader && !F.getReturnType()->isVoidTy())
      ValCtx.EmitFnFormatError(&F, ValidationRule::DeclShaderReturnVoid,
                               {F.getName()});

    auto ArgFormatError = [&](Function &F, Argument &arg, ValidationRule rule) {
      if (arg.hasName())
        ValCtx.EmitFnFormatError(&F, rule, {arg.getName().str(), F.getName()});
      else
        ValCtx.EmitFnFormatError(&F, rule,
                                 {std::to_string(arg.getArgNo()), F.getName()});
    };

    unsigned numArgs = 0;
    for (auto &arg : F.args()) {
      Type *argTy = arg.getType();
      if (argTy->isPointerTy())
        argTy = argTy->getPointerElementType();

      numArgs++;
      if (numUDTShaderArgs) {
        if (arg.getArgNo() >= numUDTShaderArgs) {
          ArgFormatError(F, arg, ValidationRule::DeclExtraArgs);
        } else if (!argTy->isStructTy()) {
          switch (shaderKind) {
          case DXIL::ShaderKind::Callable:
            ArgFormatError(F, arg, ValidationRule::DeclParamStruct);
            break;
          default:
            ArgFormatError(F, arg,
                           arg.getArgNo() == 0
                               ? ValidationRule::DeclPayloadStruct
                               : ValidationRule::DeclAttrStruct);
          }
        }
        continue;
      }

      while (argTy->isArrayTy()) {
        argTy = argTy->getArrayElementType();
      }

      if (argTy->isStructTy() && !ValCtx.isLibProfile) {
        ArgFormatError(F, arg, ValidationRule::DeclFnFlattenParam);
        break;
      }
    }

    if (numArgs < numUDTShaderArgs && shaderKind != DXIL::ShaderKind::Node) {
      StringRef argType[2] = {
          shaderKind == DXIL::ShaderKind::Callable ? "params" : "payload",
          "attributes"};
      for (unsigned i = numArgs; i < numUDTShaderArgs; i++) {
        ValCtx.EmitFnFormatError(
            &F, ValidationRule::DeclShaderMissingArg,
            {ShaderModel::GetKindName(shaderKind), F.getName(), argType[i]});
      }
    }

    if (ValCtx.DxilMod.HasDxilFunctionProps(&F) &&
        ValCtx.DxilMod.GetDxilFunctionProps(&F).IsNode()) {
      ValidateNodeInputRecord(&F, ValCtx);
    }

    ValidateFunctionBody(&F, ValCtx);
  }

  // function params & return type must not contain resources
  if (dxilutil::ContainsHLSLObjectType(F.getReturnType())) {
    ValCtx.EmitFnFormatError(&F, ValidationRule::DeclResourceInFnSig,
                             {F.getName()});
    return;
  }
  for (auto &Arg : F.args()) {
    if (dxilutil::ContainsHLSLObjectType(Arg.getType())) {
      ValCtx.EmitFnFormatError(&F, ValidationRule::DeclResourceInFnSig,
                               {F.getName()});
      return;
    }
  }

  // TODO: Remove attribute for lib?
  if (!ValCtx.isLibProfile)
    ValidateFunctionAttribute(&F, ValCtx);

  if (F.hasMetadata()) {
    ValidateFunctionMetadata(&F, ValCtx);
  }
}

static void ValidateGlobalVariable(GlobalVariable &GV,
                                   ValidationContext &ValCtx) {
  bool isInternalGV =
      dxilutil::IsStaticGlobal(&GV) || dxilutil::IsSharedMemoryGlobal(&GV);

  if (ValCtx.isLibProfile) {
    auto isCBufferGlobal =
        [&](const std::vector<std::unique_ptr<DxilCBuffer>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };
    auto isResourceGlobal =
        [&](const std::vector<std::unique_ptr<DxilResource>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };
    auto isSamplerGlobal =
        [&](const std::vector<std::unique_ptr<DxilSampler>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };

    bool isRes = isCBufferGlobal(ValCtx.DxilMod.GetCBuffers());
    isRes |= isResourceGlobal(ValCtx.DxilMod.GetUAVs());
    isRes |= isResourceGlobal(ValCtx.DxilMod.GetSRVs());
    isRes |= isSamplerGlobal(ValCtx.DxilMod.GetSamplers());
    isInternalGV |= isRes;

    // Allow special dx.ishelper for library target
    if (GV.getName().compare(DXIL::kDxIsHelperGlobalName) == 0) {
      Type *Ty = GV.getType()->getPointerElementType();
      if (Ty->isIntegerTy() && Ty->getScalarSizeInBits() == 32) {
        isInternalGV = true;
      }
    }
  }

  if (!isInternalGV) {
    if (!GV.user_empty()) {
      bool hasInstructionUser = false;
      for (User *U : GV.users()) {
        if (isa<Instruction>(U)) {
          hasInstructionUser = true;
          break;
        }
      }
      // External GV should not have instruction user.
      if (hasInstructionUser) {
        ValCtx.EmitGlobalVariableFormatError(
            &GV, ValidationRule::DeclNotUsedExternal, {GV.getName()});
      }
    }
    // Must have metadata description for each variable.

  } else {
    // Internal GV must have user.
    if (GV.user_empty()) {
      ValCtx.EmitGlobalVariableFormatError(
          &GV, ValidationRule::DeclUsedInternal, {GV.getName()});
    }

    // Validate type for internal globals.
    if (dxilutil::IsStaticGlobal(&GV) || dxilutil::IsSharedMemoryGlobal(&GV)) {
      Type *Ty = GV.getType()->getPointerElementType();
      ValidateType(Ty, ValCtx);
    }
  }
}

static void CollectFixAddressAccess(Value *V,
                                    std::vector<StoreInst *> &fixAddrTGSMList) {
  for (User *U : V->users()) {
    if (GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
      if (isa<ConstantExpr>(GEP) || GEP->hasAllConstantIndices()) {
        CollectFixAddressAccess(GEP, fixAddrTGSMList);
      }
    } else if (StoreInst *SI = dyn_cast<StoreInst>(U)) {
      fixAddrTGSMList.emplace_back(SI);
    }
  }
}

static bool IsDivergent(Value *V) {
  // TODO: return correct result.
  return false;
}

static void ValidateTGSMRaceCondition(std::vector<StoreInst *> &fixAddrTGSMList,
                                      ValidationContext &ValCtx) {
  std::unordered_set<Function *> fixAddrTGSMFuncSet;
  for (StoreInst *I : fixAddrTGSMList) {
    BasicBlock *BB = I->getParent();
    fixAddrTGSMFuncSet.insert(BB->getParent());
  }

  for (auto &F : ValCtx.DxilMod.GetModule()->functions()) {
    if (F.isDeclaration() || !fixAddrTGSMFuncSet.count(&F))
      continue;

    PostDominatorTree PDT;
    PDT.runOnFunction(F);

    BasicBlock *Entry = &F.getEntryBlock();

    for (StoreInst *SI : fixAddrTGSMList) {
      BasicBlock *BB = SI->getParent();
      if (BB->getParent() == &F) {
        if (PDT.dominates(BB, Entry)) {
          if (IsDivergent(SI->getValueOperand()))
            ValCtx.EmitInstrError(SI, ValidationRule::InstrTGSMRaceCond);
        }
      }
    }
  }
}

static void ValidateGlobalVariables(ValidationContext &ValCtx) {
  DxilModule &M = ValCtx.DxilMod;

  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
  bool TGSMAllowed = pSM->IsCS() || pSM->IsAS() || pSM->IsMS() || pSM->IsLib();

  unsigned TGSMSize = 0;
  std::vector<StoreInst *> fixAddrTGSMList;
  const DataLayout &DL = M.GetModule()->getDataLayout();
  for (GlobalVariable &GV : M.GetModule()->globals()) {
    ValidateGlobalVariable(GV, ValCtx);
    if (GV.getType()->getAddressSpace() == DXIL::kTGSMAddrSpace) {
      if (!TGSMAllowed)
        ValCtx.EmitGlobalVariableFormatError(
            &GV, ValidationRule::SmTGSMUnsupported,
            {std::string("in Shader Model ") + M.GetShaderModel()->GetName()});
      // Lib targets need to check the usage to know if it's allowed
      if (pSM->IsLib()) {
        for (User *U : GV.users()) {
          if (Instruction *I = dyn_cast<Instruction>(U)) {
            llvm::Function *F = I->getParent()->getParent();
            if (M.HasDxilEntryProps(F)) {
              DxilFunctionProps &props = M.GetDxilEntryProps(F).props;
              if (!props.IsCS() && !props.IsAS() && !props.IsMS() &&
                  !props.IsNode()) {
                ValCtx.EmitInstrFormatError(I,
                                            ValidationRule::SmTGSMUnsupported,
                                            {"from non-compute entry points"});
              }
            }
          }
        }
      }
      TGSMSize += DL.getTypeAllocSize(GV.getType()->getElementType());
      CollectFixAddressAccess(&GV, fixAddrTGSMList);
    }
  }

  ValidationRule Rule = ValidationRule::SmMaxTGSMSize;
  unsigned MaxSize = DXIL::kMaxTGSMSize;

  if (M.GetShaderModel()->IsMS()) {
    Rule = ValidationRule::SmMaxMSSMSize;
    MaxSize = DXIL::kMaxMSSMSize;
  }
  if (TGSMSize > MaxSize) {
    Module::global_iterator GI = M.GetModule()->global_end();
    GlobalVariable *GV = &*GI;
    do {
      GI--;
      GV = &*GI;
      if (GV->getType()->getAddressSpace() == hlsl::DXIL::kTGSMAddrSpace)
        break;
    } while (GI != M.GetModule()->global_begin());
    ValCtx.EmitGlobalVariableFormatError(
        GV, Rule, {std::to_string(TGSMSize), std::to_string(MaxSize)});
  }

  if (!fixAddrTGSMList.empty()) {
    ValidateTGSMRaceCondition(fixAddrTGSMList, ValCtx);
  }
}

static void ValidateValidatorVersion(ValidationContext &ValCtx) {
  Module *pModule = &ValCtx.M;
  NamedMDNode *pNode = pModule->getNamedMetadata("dx.valver");
  if (pNode == nullptr) {
    return;
  }
  if (pNode->getNumOperands() == 1) {
    MDTuple *pVerValues = dyn_cast<MDTuple>(pNode->getOperand(0));
    if (pVerValues != nullptr && pVerValues->getNumOperands() == 2) {
      uint64_t majorVer, minorVer;
      if (GetNodeOperandAsInt(ValCtx, pVerValues, 0, &majorVer) &&
          GetNodeOperandAsInt(ValCtx, pVerValues, 1, &minorVer)) {
        unsigned curMajor, curMinor;
        GetValidationVersion(&curMajor, &curMinor);
        // This will need to be updated as major/minor versions evolve,
        // depending on the degree of compat across versions.
        if (majorVer == curMajor && minorVer <= curMinor) {
          return;
        } else {
          ValCtx.EmitFormatError(
              ValidationRule::MetaVersionSupported,
              {"Validator", std::to_string(majorVer), std::to_string(minorVer),
               std::to_string(curMajor), std::to_string(curMinor)});
          return;
        }
      }
    }
  }
  ValCtx.EmitError(ValidationRule::MetaWellFormed);
}

static void ValidateDxilVersion(ValidationContext &ValCtx) {
  Module *pModule = &ValCtx.M;
  NamedMDNode *pNode = pModule->getNamedMetadata("dx.version");
  if (pNode == nullptr) {
    return;
  }
  if (pNode->getNumOperands() == 1) {
    MDTuple *pVerValues = dyn_cast<MDTuple>(pNode->getOperand(0));
    if (pVerValues != nullptr && pVerValues->getNumOperands() == 2) {
      uint64_t majorVer, minorVer;
      if (GetNodeOperandAsInt(ValCtx, pVerValues, 0, &majorVer) &&
          GetNodeOperandAsInt(ValCtx, pVerValues, 1, &minorVer)) {
        // This will need to be updated as dxil major/minor versions evolve,
        // depending on the degree of compat across versions.
        if ((majorVer == DXIL::kDxilMajor && minorVer <= DXIL::kDxilMinor) &&
            (majorVer == ValCtx.m_DxilMajor &&
             minorVer == ValCtx.m_DxilMinor)) {
          return;
        } else {
          ValCtx.EmitFormatError(ValidationRule::MetaVersionSupported,
                                 {"Dxil", std::to_string(majorVer),
                                  std::to_string(minorVer),
                                  std::to_string(DXIL::kDxilMajor),
                                  std::to_string(DXIL::kDxilMinor)});
          return;
        }
      }
    }
  }
  // ValCtx.EmitMetaError(pNode, ValidationRule::MetaWellFormed);
  ValCtx.EmitError(ValidationRule::MetaWellFormed);
}

static void ValidateTypeAnnotation(ValidationContext &ValCtx) {
  if (ValCtx.m_DxilMajor == 1 && ValCtx.m_DxilMinor >= 2) {
    Module *pModule = &ValCtx.M;
    NamedMDNode *TA = pModule->getNamedMetadata("dx.typeAnnotations");
    if (TA == nullptr)
      return;
    for (unsigned i = 0, end = TA->getNumOperands(); i < end; ++i) {
      MDTuple *TANode = dyn_cast<MDTuple>(TA->getOperand(i));
      if (TANode->getNumOperands() < 3) {
        ValCtx.EmitMetaError(TANode, ValidationRule::MetaWellFormed);
        return;
      }
      ConstantInt *tag = mdconst::extract<ConstantInt>(TANode->getOperand(0));
      uint64_t tagValue = tag->getZExtValue();
      if (tagValue != DxilMDHelper::kDxilTypeSystemStructTag &&
          tagValue != DxilMDHelper::kDxilTypeSystemFunctionTag) {
        ValCtx.EmitMetaError(TANode, ValidationRule::MetaWellFormed);
        return;
      }
    }
  }
}

static void ValidateBitcode(ValidationContext &ValCtx) {
  std::string diagStr;
  raw_string_ostream diagStream(diagStr);
  if (llvm::verifyModule(ValCtx.M, &diagStream)) {
    ValCtx.EmitError(ValidationRule::BitcodeValid);
    dxilutil::EmitErrorOnContext(ValCtx.M.getContext(), diagStream.str());
  }
}

static void ValidateWaveSize(ValidationContext &ValCtx,
                             const hlsl::ShaderModel *SM, Module *pModule) {
  // Don't do this validation if the shader is non-compute
  if (!(SM->IsCS() || SM->IsLib()))
    return;

  NamedMDNode *EPs = pModule->getNamedMetadata("dx.entryPoints");
  if (!EPs)
    return;

  for (unsigned i = 0, end = EPs->getNumOperands(); i < end; ++i) {
    MDTuple *EPNodeRef = dyn_cast<MDTuple>(EPs->getOperand(i));
    if (EPNodeRef->getNumOperands() < 5) {
      ValCtx.EmitMetaError(EPNodeRef, ValidationRule::MetaWellFormed);
      return;
    }
    // get access to the digit that represents the metadata number that
    // would store entry properties
    const llvm::MDOperand &mOp =
        EPNodeRef->getOperand(EPNodeRef->getNumOperands() - 1);
    // the final operand to the entry points tuple should be a tuple.
    if (mOp == nullptr || (mOp.get())->getMetadataID() != Metadata::MDTupleKind)
      continue;

    // get access to the node that stores entry properties
    MDTuple *EPropNode = dyn_cast<MDTuple>(
        EPNodeRef->getOperand(EPNodeRef->getNumOperands() - 1));
    // find any incompatible tags inside the entry properties
    // increment j by 2 to only analyze tags, not values
    bool foundTag = false;
    for (unsigned j = 0, end2 = EPropNode->getNumOperands(); j < end2; j += 2) {
      const MDOperand &propertyTagOp = EPropNode->getOperand(j);
      // note, we are only looking for tags, which will be a constant
      // integer
      DXASSERT(!(propertyTagOp == nullptr ||
                 (propertyTagOp.get())->getMetadataID() !=
                     Metadata::ConstantAsMetadataKind),
               "tag operand should be a constant integer.");

      ConstantInt *tag = mdconst::extract<ConstantInt>(propertyTagOp);
      uint64_t tagValue = tag->getZExtValue();

      // legacy wavesize is only supported between 6.6 and 6.7, so we
      // should fail if we find the ranged wave size metadata tag
      if (tagValue == DxilMDHelper::kDxilRangedWaveSizeTag) {
        // if this tag is already present in the
        // current entry point, emit an error
        if (foundTag) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeTagDuplicate, {});
          return;
        }
        foundTag = true;
        if (SM->IsSM66Plus() && !SM->IsSM68Plus()) {

          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeRangeNeedsSM68Plus,
                                 {});
          return;
        }
        // get the metadata that contains the
        // parameters to the wavesize attribute
        MDTuple *WaveTuple = dyn_cast<MDTuple>(EPropNode->getOperand(j + 1));
        if (WaveTuple->getNumOperands() != 3) {
          ValCtx.EmitFormatError(
              ValidationRule::SmWaveSizeRangeExpectsThreeParams, {});
          return;
        }
        for (int k = 0; k < 3; k++) {
          const MDOperand &param = WaveTuple->getOperand(k);
          if (param->getMetadataID() != Metadata::ConstantAsMetadataKind) {
            ValCtx.EmitFormatError(
                ValidationRule::SmWaveSizeNeedsConstantOperands, {});
            return;
          }
        }

      } else if (tagValue == DxilMDHelper::kDxilWaveSizeTag) {
        // if this tag is already present in the
        // current entry point, emit an error
        if (foundTag) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeTagDuplicate, {});
          return;
        }
        foundTag = true;
        MDTuple *WaveTuple = dyn_cast<MDTuple>(EPropNode->getOperand(j + 1));
        if (WaveTuple->getNumOperands() != 1) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeExpectsOneParam, {});
          return;
        }
        const MDOperand &param = WaveTuple->getOperand(0);
        if (param->getMetadataID() != Metadata::ConstantAsMetadataKind) {
          ValCtx.EmitFormatError(
              ValidationRule::SmWaveSizeNeedsConstantOperands, {});
          return;
        }
        // if the shader model is anything but 6.6 or 6.7, then we do not
        // expect to encounter the legacy wave size tag.
        if (!(SM->IsSM66Plus() && !SM->IsSM68Plus())) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeNeedsSM66or67, {});
          return;
        }
      }
    }
  }
}

static void ValidateMetadata(ValidationContext &ValCtx) {
  ValidateValidatorVersion(ValCtx);
  ValidateDxilVersion(ValCtx);

  Module *pModule = &ValCtx.M;
  const std::string &target = pModule->getTargetTriple();
  if (target != "dxil-ms-dx") {
    ValCtx.EmitFormatError(ValidationRule::MetaTarget, {target});
  }

  // The llvm.dbg.(cu/contents/defines/mainFileName/arg) named metadata nodes
  // are only available in debug modules, not in the validated ones.
  // llvm.bitsets is also disallowed.
  //
  // These are verified in lib/IR/Verifier.cpp.
  StringMap<bool> llvmNamedMeta;
  llvmNamedMeta["llvm.ident"];
  llvmNamedMeta["llvm.module.flags"];

  for (auto &NamedMetaNode : pModule->named_metadata()) {
    if (!DxilModule::IsKnownNamedMetaData(NamedMetaNode)) {
      StringRef name = NamedMetaNode.getName();
      if (!name.startswith_lower("llvm.")) {
        ValCtx.EmitFormatError(ValidationRule::MetaKnown, {name.str()});
      } else {
        if (llvmNamedMeta.count(name) == 0) {
          ValCtx.EmitFormatError(ValidationRule::MetaKnown, {name.str()});
        }
      }
    }
  }

  const hlsl::ShaderModel *SM = ValCtx.DxilMod.GetShaderModel();
  // validate that any wavesize tags don't appear outside their expected shader
  // models. Validate only 1 tag exists per entry point.
  ValidateWaveSize(ValCtx, SM, pModule);

  if (!SM->IsValidForDxil()) {
    ValCtx.EmitFormatError(ValidationRule::SmName,
                           {ValCtx.DxilMod.GetShaderModel()->GetName()});
  }

  if (SM->GetMajor() == 6) {
    // Make sure DxilVersion matches the shader model.
    unsigned SMDxilMajor, SMDxilMinor;
    SM->GetDxilVersion(SMDxilMajor, SMDxilMinor);
    if (ValCtx.m_DxilMajor != SMDxilMajor ||
        ValCtx.m_DxilMinor != SMDxilMinor) {
      ValCtx.EmitFormatError(
          ValidationRule::SmDxilVersion,
          {std::to_string(SMDxilMajor), std::to_string(SMDxilMinor)});
    }
  }

  ValidateTypeAnnotation(ValCtx);
}

static void ValidateResourceOverlap(
    hlsl::DxilResourceBase &res,
    SpacesAllocator<unsigned, DxilResourceBase> &spaceAllocator,
    ValidationContext &ValCtx) {
  unsigned base = res.GetLowerBound();
  if (ValCtx.isLibProfile && !res.IsAllocated()) {
    // Skip unallocated resource for library.
    return;
  }
  unsigned size = res.GetRangeSize();
  unsigned space = res.GetSpaceID();

  auto &allocator = spaceAllocator.Get(space);
  unsigned end = base + size - 1;
  // unbounded
  if (end < base)
    end = size;
  const DxilResourceBase *conflictRes = allocator.Insert(&res, base, end);
  if (conflictRes) {
    ValCtx.EmitFormatError(
        ValidationRule::SmResourceRangeOverlap,
        {ValCtx.GetResourceName(&res), std::to_string(base),
         std::to_string(size), std::to_string(conflictRes->GetLowerBound()),
         std::to_string(conflictRes->GetRangeSize()), std::to_string(space)});
  }
}

static void ValidateResource(hlsl::DxilResource &res,
                             ValidationContext &ValCtx) {
  switch (res.GetKind()) {
  case DXIL::ResourceKind::RawBuffer:
  case DXIL::ResourceKind::TypedBuffer:
  case DXIL::ResourceKind::TBuffer:
  case DXIL::ResourceKind::StructuredBuffer:
  case DXIL::ResourceKind::Texture1D:
  case DXIL::ResourceKind::Texture1DArray:
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture2DArray:
  case DXIL::ResourceKind::Texture3D:
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::TextureCubeArray:
    if (res.GetSampleCount() > 0) {
      ValCtx.EmitResourceError(&res, ValidationRule::SmSampleCountOnlyOn2DMS);
    }
    break;
  case DXIL::ResourceKind::Texture2DMS:
  case DXIL::ResourceKind::Texture2DMSArray:
    break;
  case DXIL::ResourceKind::RTAccelerationStructure:
    // TODO: check profile.
    break;
  case DXIL::ResourceKind::FeedbackTexture2D:
  case DXIL::ResourceKind::FeedbackTexture2DArray:
    if (res.GetSamplerFeedbackType() >= DXIL::SamplerFeedbackType::LastEntry)
      ValCtx.EmitResourceError(&res,
                               ValidationRule::SmInvalidSamplerFeedbackType);
    break;
  default:
    ValCtx.EmitResourceError(&res, ValidationRule::SmInvalidResourceKind);
    break;
  }

  switch (res.GetCompType().GetKind()) {
  case DXIL::ComponentType::F32:
  case DXIL::ComponentType::SNormF32:
  case DXIL::ComponentType::UNormF32:
  case DXIL::ComponentType::F64:
  case DXIL::ComponentType::I32:
  case DXIL::ComponentType::I64:
  case DXIL::ComponentType::U32:
  case DXIL::ComponentType::U64:
  case DXIL::ComponentType::F16:
  case DXIL::ComponentType::I16:
  case DXIL::ComponentType::U16:
    break;
  default:
    if (!res.IsStructuredBuffer() && !res.IsRawBuffer() &&
        !res.IsFeedbackTexture())
      ValCtx.EmitResourceError(&res, ValidationRule::SmInvalidResourceCompType);
    break;
  }

  if (res.IsStructuredBuffer()) {
    unsigned stride = res.GetElementStride();
    bool alignedTo4Bytes = (stride & 3) == 0;
    if (!alignedTo4Bytes && ValCtx.M.GetDxilModule().GetUseMinPrecision()) {
      ValCtx.EmitResourceFormatError(
          &res, ValidationRule::MetaStructBufAlignment,
          {std::to_string(4), std::to_string(stride)});
    }
    if (stride > DXIL::kMaxStructBufferStride) {
      ValCtx.EmitResourceFormatError(
          &res, ValidationRule::MetaStructBufAlignmentOutOfBound,
          {std::to_string(DXIL::kMaxStructBufferStride),
           std::to_string(stride)});
    }
  }

  if (res.IsAnyTexture() || res.IsTypedBuffer()) {
    Type *RetTy = res.GetRetType();
    unsigned size =
        ValCtx.DxilMod.GetModule()->getDataLayout().getTypeAllocSize(RetTy);
    if (size > 4 * 4) {
      ValCtx.EmitResourceError(&res, ValidationRule::MetaTextureType);
    }
  }
}

static void CollectCBufferRanges(
    DxilStructAnnotation *annotation,
    SpanAllocator<unsigned, DxilFieldAnnotation> &constAllocator, unsigned base,
    DxilTypeSystem &typeSys, StringRef cbName, ValidationContext &ValCtx) {
  DXASSERT(((base + 15) & ~(0xf)) == base,
           "otherwise, base for struct is not aligned");
  unsigned cbSize = annotation->GetCBufferSize();

  const StructType *ST = annotation->GetStructType();

  for (int i = annotation->GetNumFields() - 1; i >= 0; i--) {
    DxilFieldAnnotation &fieldAnnotation = annotation->GetFieldAnnotation(i);
    Type *EltTy = ST->getElementType(i);

    unsigned offset = fieldAnnotation.GetCBufferOffset();

    unsigned EltSize = dxilutil::GetLegacyCBufferFieldElementSize(
        fieldAnnotation, EltTy, typeSys);

    bool bOutOfBound = false;
    if (!EltTy->isAggregateType()) {
      bOutOfBound = (offset + EltSize) > cbSize;
      if (!bOutOfBound) {
        if (constAllocator.Insert(&fieldAnnotation, base + offset,
                                  base + offset + EltSize - 1)) {
          ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                 {cbName, std::to_string(base + offset)});
        }
      }
    } else if (isa<ArrayType>(EltTy)) {
      if (((offset + 15) & ~(0xf)) != offset) {
        ValCtx.EmitFormatError(ValidationRule::SmCBufferArrayOffsetAlignment,
                               {cbName, std::to_string(offset)});
        continue;
      }
      unsigned arrayCount = 1;
      while (isa<ArrayType>(EltTy)) {
        arrayCount *= EltTy->getArrayNumElements();
        EltTy = EltTy->getArrayElementType();
      }

      DxilStructAnnotation *EltAnnotation = nullptr;
      if (StructType *EltST = dyn_cast<StructType>(EltTy))
        EltAnnotation = typeSys.GetStructAnnotation(EltST);

      unsigned alignedEltSize = ((EltSize + 15) & ~(0xf));
      unsigned arraySize = ((arrayCount - 1) * alignedEltSize) + EltSize;
      bOutOfBound = (offset + arraySize) > cbSize;

      if (!bOutOfBound) {
        // If we didn't care about gaps where elements could be placed with user
        // offsets, we could: recurse once if EltAnnotation, then allocate the
        // rest if arrayCount > 1

        unsigned arrayBase = base + offset;
        if (!EltAnnotation) {
          if (EltSize > 0 &&
              nullptr != constAllocator.Insert(&fieldAnnotation, arrayBase,
                                               arrayBase + arraySize - 1)) {
            ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                   {cbName, std::to_string(arrayBase)});
          }
        } else {
          for (unsigned idx = 0; idx < arrayCount; idx++) {
            CollectCBufferRanges(EltAnnotation, constAllocator, arrayBase,
                                 typeSys, cbName, ValCtx);
            arrayBase += alignedEltSize;
          }
        }
      }
    } else {
      StructType *EltST = cast<StructType>(EltTy);
      unsigned structBase = base + offset;
      bOutOfBound = (offset + EltSize) > cbSize;
      if (!bOutOfBound) {
        if (DxilStructAnnotation *EltAnnotation =
                typeSys.GetStructAnnotation(EltST)) {
          CollectCBufferRanges(EltAnnotation, constAllocator, structBase,
                               typeSys, cbName, ValCtx);
        } else {
          if (EltSize > 0 &&
              nullptr != constAllocator.Insert(&fieldAnnotation, structBase,
                                               structBase + EltSize - 1)) {
            ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                   {cbName, std::to_string(structBase)});
          }
        }
      }
    }

    if (bOutOfBound) {
      ValCtx.EmitFormatError(ValidationRule::SmCBufferElementOverflow,
                             {cbName, std::to_string(base + offset)});
    }
  }
}

static void ValidateCBuffer(DxilCBuffer &cb, ValidationContext &ValCtx) {
  Type *Ty = cb.GetHLSLType()->getPointerElementType();
  if (cb.GetRangeSize() != 1 || Ty->isArrayTy()) {
    Ty = Ty->getArrayElementType();
  }
  if (!isa<StructType>(Ty)) {
    ValCtx.EmitResourceError(&cb,
                             ValidationRule::SmCBufferTemplateTypeMustBeStruct);
    return;
  }
  if (cb.GetSize() > (DXIL::kMaxCBufferSize << 4)) {
    ValCtx.EmitResourceFormatError(&cb, ValidationRule::SmCBufferSize,
                                   {std::to_string(cb.GetSize())});
    return;
  }
  StructType *ST = cast<StructType>(Ty);
  DxilTypeSystem &typeSys = ValCtx.DxilMod.GetTypeSystem();
  DxilStructAnnotation *annotation = typeSys.GetStructAnnotation(ST);
  if (!annotation)
    return;

  // Collect constant ranges.
  std::vector<std::pair<unsigned, unsigned>> constRanges;
  SpanAllocator<unsigned, DxilFieldAnnotation> constAllocator(
      0,
      // 4096 * 16 bytes.
      DXIL::kMaxCBufferSize << 4);
  CollectCBufferRanges(annotation, constAllocator, 0, typeSys,
                       ValCtx.GetResourceName(&cb), ValCtx);
}

static void ValidateResources(ValidationContext &ValCtx) {
  const vector<unique_ptr<DxilResource>> &uavs = ValCtx.DxilMod.GetUAVs();
  SpacesAllocator<unsigned, DxilResourceBase> uavAllocator;

  for (auto &uav : uavs) {
    if (uav->IsROV()) {
      if (!ValCtx.DxilMod.GetShaderModel()->IsPS() && !ValCtx.isLibProfile) {
        ValCtx.EmitResourceError(uav.get(), ValidationRule::SmROVOnlyInPS);
      }
    }
    switch (uav->GetKind()) {
    case DXIL::ResourceKind::TextureCube:
    case DXIL::ResourceKind::TextureCubeArray:
      ValCtx.EmitResourceError(uav.get(),
                               ValidationRule::SmInvalidTextureKindOnUAV);
      break;
    default:
      break;
    }

    if (uav->HasCounter() && !uav->IsStructuredBuffer()) {
      ValCtx.EmitResourceError(uav.get(),
                               ValidationRule::SmCounterOnlyOnStructBuf);
    }
    if (uav->HasCounter() && uav->IsGloballyCoherent())
      ValCtx.EmitResourceFormatError(uav.get(),
                                     ValidationRule::MetaGlcNotOnAppendConsume,
                                     {ValCtx.GetResourceName(uav.get())});

    ValidateResource(*uav, ValCtx);
    ValidateResourceOverlap(*uav, uavAllocator, ValCtx);
  }

  SpacesAllocator<unsigned, DxilResourceBase> srvAllocator;
  const vector<unique_ptr<DxilResource>> &srvs = ValCtx.DxilMod.GetSRVs();
  for (auto &srv : srvs) {
    ValidateResource(*srv, ValCtx);
    ValidateResourceOverlap(*srv, srvAllocator, ValCtx);
  }

  hlsl::DxilResourceBase *pNonDense;
  if (!AreDxilResourcesDense(&ValCtx.M, &pNonDense)) {
    ValCtx.EmitResourceError(pNonDense, ValidationRule::MetaDenseResIDs);
  }

  SpacesAllocator<unsigned, DxilResourceBase> samplerAllocator;
  for (auto &sampler : ValCtx.DxilMod.GetSamplers()) {
    if (sampler->GetSamplerKind() == DXIL::SamplerKind::Invalid) {
      ValCtx.EmitResourceError(sampler.get(),
                               ValidationRule::MetaValidSamplerMode);
    }
    ValidateResourceOverlap(*sampler, samplerAllocator, ValCtx);
  }

  SpacesAllocator<unsigned, DxilResourceBase> cbufferAllocator;
  for (auto &cbuffer : ValCtx.DxilMod.GetCBuffers()) {
    ValidateCBuffer(*cbuffer, ValCtx);
    ValidateResourceOverlap(*cbuffer, cbufferAllocator, ValCtx);
  }
}

static void ValidateShaderFlags(ValidationContext &ValCtx) {
  ShaderFlags calcFlags;
  ValCtx.DxilMod.CollectShaderFlagsForModule(calcFlags);

  // Special case for validator version prior to 1.8.
  // If DXR 1.1 flag is set, but our computed flags do not have this set, then
  // this is due to prior versions setting the flag based on DXR 1.1 subobjects,
  // which are gone by this point.  Set the flag and the rest should match.
  unsigned valMajor, valMinor;
  ValCtx.DxilMod.GetValidatorVersion(valMajor, valMinor);
  if (DXIL::CompareVersions(valMajor, valMinor, 1, 5) >= 0 &&
      DXIL::CompareVersions(valMajor, valMinor, 1, 8) < 0 &&
      ValCtx.DxilMod.m_ShaderFlags.GetRaytracingTier1_1() &&
      !calcFlags.GetRaytracingTier1_1()) {
    calcFlags.SetRaytracingTier1_1(true);
  }

  const uint64_t mask = ShaderFlags::GetShaderFlagsRawForCollection();
  uint64_t declaredFlagsRaw = ValCtx.DxilMod.m_ShaderFlags.GetShaderFlagsRaw();
  uint64_t calcFlagsRaw = calcFlags.GetShaderFlagsRaw();

  declaredFlagsRaw &= mask;
  calcFlagsRaw &= mask;

  if (declaredFlagsRaw == calcFlagsRaw) {
    return;
  }
  ValCtx.EmitError(ValidationRule::MetaFlagsUsage);

  dxilutil::EmitNoteOnContext(ValCtx.M.getContext(),
                              Twine("Flags declared=") +
                                  Twine(declaredFlagsRaw) + Twine(", actual=") +
                                  Twine(calcFlagsRaw));
}

static void ValidateSignatureElement(DxilSignatureElement &SE,
                                     ValidationContext &ValCtx) {
  DXIL::SemanticKind semanticKind = SE.GetSemantic()->GetKind();
  CompType::Kind compKind = SE.GetCompType().GetKind();
  DXIL::InterpolationMode Mode = SE.GetInterpolationMode()->GetKind();

  StringRef Name = SE.GetName();
  if (Name.size() < 1 || Name.size() > 64) {
    ValCtx.EmitSignatureError(&SE, ValidationRule::MetaSemanticLen);
  }

  if (semanticKind > DXIL::SemanticKind::Arbitrary &&
      semanticKind < DXIL::SemanticKind::Invalid) {
    if (semanticKind != Semantic::GetByName(SE.GetName())->GetKind()) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemaKindMatchesName,
                             {SE.GetName(), SE.GetSemantic()->GetName()});
    }
  }

  unsigned compWidth = 0;
  bool compFloat = false;
  bool compInt = false;
  bool compBool = false;

  switch (compKind) {
  case CompType::Kind::U64:
    compWidth = 64;
    compInt = true;
    break;
  case CompType::Kind::I64:
    compWidth = 64;
    compInt = true;
    break;
  // These should be translated for signatures:
  // case CompType::Kind::PackedS8x32:
  // case CompType::Kind::PackedU8x32:
  case CompType::Kind::U32:
    compWidth = 32;
    compInt = true;
    break;
  case CompType::Kind::I32:
    compWidth = 32;
    compInt = true;
    break;
  case CompType::Kind::U16:
    compWidth = 16;
    compInt = true;
    break;
  case CompType::Kind::I16:
    compWidth = 16;
    compInt = true;
    break;
  case CompType::Kind::I1:
    compWidth = 1;
    compBool = true;
    break;
  case CompType::Kind::F64:
    compWidth = 64;
    compFloat = true;
    break;
  case CompType::Kind::F32:
    compWidth = 32;
    compFloat = true;
    break;
  case CompType::Kind::F16:
    compWidth = 16;
    compFloat = true;
    break;
  case CompType::Kind::SNormF64:
    compWidth = 64;
    compFloat = true;
    break;
  case CompType::Kind::SNormF32:
    compWidth = 32;
    compFloat = true;
    break;
  case CompType::Kind::SNormF16:
    compWidth = 16;
    compFloat = true;
    break;
  case CompType::Kind::UNormF64:
    compWidth = 64;
    compFloat = true;
    break;
  case CompType::Kind::UNormF32:
    compWidth = 32;
    compFloat = true;
    break;
  case CompType::Kind::UNormF16:
    compWidth = 16;
    compFloat = true;
    break;
  case CompType::Kind::Invalid:
  default:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureCompType,
                           {SE.GetName()});
    break;
  }

  if (compInt || compBool) {
    switch (Mode) {
    case DXIL::InterpolationMode::Linear:
    case DXIL::InterpolationMode::LinearCentroid:
    case DXIL::InterpolationMode::LinearNoperspective:
    case DXIL::InterpolationMode::LinearNoperspectiveCentroid:
    case DXIL::InterpolationMode::LinearSample:
    case DXIL::InterpolationMode::LinearNoperspectiveSample: {
      ValCtx.EmitFormatError(ValidationRule::MetaIntegerInterpMode,
                             {SE.GetName()});
    } break;
    default:
      break;
    }
  }

  // Elements that should not appear in the Dxil signature:
  bool bAllowedInSig = true;
  bool bShouldBeAllocated = true;
  switch (SE.GetInterpretation()) {
  case DXIL::SemanticInterpretationKind::NA:
  case DXIL::SemanticInterpretationKind::NotInSig:
  case DXIL::SemanticInterpretationKind::Invalid:
    bAllowedInSig = false;
    LLVM_FALLTHROUGH;
  case DXIL::SemanticInterpretationKind::NotPacked:
  case DXIL::SemanticInterpretationKind::Shadow:
    bShouldBeAllocated = false;
    break;
  default:
    break;
  }

  const char *inputOutput = nullptr;
  if (SE.IsInput())
    inputOutput = "Input";
  else if (SE.IsOutput())
    inputOutput = "Output";
  else
    inputOutput = "PatchConstant";

  if (!bAllowedInSig) {
    ValCtx.EmitFormatError(ValidationRule::SmSemantic,
                           {SE.GetName(),
                            ValCtx.DxilMod.GetShaderModel()->GetKindName(),
                            inputOutput});
  } else if (bShouldBeAllocated && !SE.IsAllocated()) {
    ValCtx.EmitFormatError(ValidationRule::MetaSemanticShouldBeAllocated,
                           {inputOutput, SE.GetName()});
  } else if (!bShouldBeAllocated && SE.IsAllocated()) {
    ValCtx.EmitFormatError(ValidationRule::MetaSemanticShouldNotBeAllocated,
                           {inputOutput, SE.GetName()});
  }

  bool bIsClipCull = false;
  bool bIsTessfactor = false;
  bool bIsBarycentric = false;

  switch (semanticKind) {
  case DXIL::SemanticKind::Depth:
  case DXIL::SemanticKind::DepthGreaterEqual:
  case DXIL::SemanticKind::DepthLessEqual:
    if (!compFloat || compWidth > 32 || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    break;
  case DXIL::SemanticKind::Coverage:
    DXASSERT(!SE.IsInput() || !bAllowedInSig,
             "else internal inconsistency between semantic interpretation "
             "table and validation code");
    LLVM_FALLTHROUGH;
  case DXIL::SemanticKind::InnerCoverage:
  case DXIL::SemanticKind::OutputControlPointID:
    if (compKind != CompType::Kind::U32 || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "uint"});
    }
    break;
  case DXIL::SemanticKind::Position:
    if (!compFloat || compWidth > 32 || SE.GetCols() != 4) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float4"});
    }
    break;
  case DXIL::SemanticKind::Target:
    if (compWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float/int/uint"});
    }
    break;
  case DXIL::SemanticKind::ClipDistance:
  case DXIL::SemanticKind::CullDistance:
    bIsClipCull = true;
    if (!compFloat || compWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    // NOTE: clip cull distance size is checked at ValidateSignature.
    break;
  case DXIL::SemanticKind::IsFrontFace: {
    if (!(compInt && compWidth == 32) || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "uint"});
    }
  } break;
  case DXIL::SemanticKind::RenderTargetArrayIndex:
  case DXIL::SemanticKind::ViewPortArrayIndex:
  case DXIL::SemanticKind::VertexID:
  case DXIL::SemanticKind::PrimitiveID:
  case DXIL::SemanticKind::InstanceID:
  case DXIL::SemanticKind::GSInstanceID:
  case DXIL::SemanticKind::SampleIndex:
  case DXIL::SemanticKind::StencilRef:
  case DXIL::SemanticKind::ShadingRate:
    if ((compKind != CompType::Kind::U32 && compKind != CompType::Kind::U16) ||
        SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "uint"});
    }
    break;
  case DXIL::SemanticKind::CullPrimitive: {
    if (!(compBool && compWidth == 1) || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "bool"});
    }
  } break;
  case DXIL::SemanticKind::TessFactor:
  case DXIL::SemanticKind::InsideTessFactor:
    // NOTE: the size check is at CheckPatchConstantSemantic.
    bIsTessfactor = true;
    if (!compFloat || compWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    break;
  case DXIL::SemanticKind::Arbitrary:
    break;
  case DXIL::SemanticKind::DomainLocation:
  case DXIL::SemanticKind::Invalid:
    DXASSERT(!bAllowedInSig, "else internal inconsistency between semantic "
                             "interpretation table and validation code");
    break;
  case DXIL::SemanticKind::Barycentrics:
    bIsBarycentric = true;
    if (!compFloat || compWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    if (Mode != InterpolationMode::Kind::Linear &&
        Mode != InterpolationMode::Kind::LinearCentroid &&
        Mode != InterpolationMode::Kind::LinearNoperspective &&
        Mode != InterpolationMode::Kind::LinearNoperspectiveCentroid &&
        Mode != InterpolationMode::Kind::LinearNoperspectiveSample &&
        Mode != InterpolationMode::Kind::LinearSample) {
      ValCtx.EmitSignatureError(&SE,
                                ValidationRule::MetaBarycentricsInterpolation);
    }
    if (SE.GetCols() != 3) {
      ValCtx.EmitSignatureError(&SE, ValidationRule::MetaBarycentricsFloat3);
    }
    break;
  default:
    ValCtx.EmitSignatureError(&SE, ValidationRule::MetaSemaKindValid);
    break;
  }

  if (ValCtx.DxilMod.GetShaderModel()->IsGS() && SE.IsOutput()) {
    if (SE.GetOutputStream() >= DXIL::kNumOutputStreams) {
      ValCtx.EmitFormatError(ValidationRule::SmStreamIndexRange,
                             {std::to_string(SE.GetOutputStream()),
                              std::to_string(DXIL::kNumOutputStreams - 1)});
    }
  } else {
    if (SE.GetOutputStream() > 0) {
      ValCtx.EmitFormatError(ValidationRule::SmStreamIndexRange,
                             {std::to_string(SE.GetOutputStream()), "0"});
    }
  }

  if (ValCtx.DxilMod.GetShaderModel()->IsGS()) {
    if (SE.GetOutputStream() != 0) {
      if (ValCtx.DxilMod.GetStreamPrimitiveTopology() !=
          DXIL::PrimitiveTopology::PointList) {
        ValCtx.EmitSignatureError(&SE,
                                  ValidationRule::SmMultiStreamMustBePoint);
      }
    }
  }

  if (semanticKind == DXIL::SemanticKind::Target) {
    // Verify packed row == semantic index
    unsigned row = SE.GetStartRow();
    for (unsigned i : SE.GetSemanticIndexVec()) {
      if (row != i) {
        ValCtx.EmitSignatureError(&SE,
                                  ValidationRule::SmPSTargetIndexMatchesRow);
      }
      ++row;
    }
    // Verify packed col is 0
    if (SE.GetStartCol() != 0) {
      ValCtx.EmitSignatureError(&SE, ValidationRule::SmPSTargetCol0);
    }
    // Verify max row used < 8
    if (SE.GetStartRow() + SE.GetRows() > 8) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                             {"SV_Target", "7"});
    }
  } else if (bAllowedInSig && semanticKind != DXIL::SemanticKind::Arbitrary) {
    if (bIsBarycentric) {
      if (SE.GetSemanticStartIndex() > 1) {
        ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                               {SE.GetSemantic()->GetName(), "1"});
      }
    } else if (!bIsClipCull && SE.GetSemanticStartIndex() > 0) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                             {SE.GetSemantic()->GetName(), "0"});
    }
    // Maximum rows is 1 for system values other than Target
    // with the exception of tessfactors, which are validated in
    // CheckPatchConstantSemantic and ClipDistance/CullDistance, which have
    // other custom constraints.
    if (!bIsTessfactor && !bIsClipCull && SE.GetRows() > 1) {
      ValCtx.EmitSignatureError(&SE, ValidationRule::MetaSystemValueRows);
    }
  }

  if (SE.GetCols() + (SE.IsAllocated() ? SE.GetStartCol() : 0) > 4) {
    unsigned size = (SE.GetRows() - 1) * 4 + SE.GetCols();
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureOutOfRange,
                           {SE.GetName(), std::to_string(SE.GetStartRow()),
                            std::to_string(SE.GetStartCol()),
                            std::to_string(size)});
  }

  if (!SE.GetInterpolationMode()->IsValid()) {
    ValCtx.EmitSignatureError(&SE, ValidationRule::MetaInterpModeValid);
  }
}

static void ValidateSignatureOverlap(DxilSignatureElement &E,
                                     unsigned maxScalars,
                                     DxilSignatureAllocator &allocator,
                                     ValidationContext &ValCtx) {

  // Skip entries that are not or should not be allocated.  Validation occurs in
  // ValidateSignatureElement.
  if (!E.IsAllocated())
    return;
  switch (E.GetInterpretation()) {
  case DXIL::SemanticInterpretationKind::NA:
  case DXIL::SemanticInterpretationKind::NotInSig:
  case DXIL::SemanticInterpretationKind::Invalid:
  case DXIL::SemanticInterpretationKind::NotPacked:
  case DXIL::SemanticInterpretationKind::Shadow:
    return;
  default:
    break;
  }

  DxilPackElement PE(&E, allocator.UseMinPrecision());
  DxilSignatureAllocator::ConflictType conflict =
      allocator.DetectRowConflict(&PE, E.GetStartRow());
  if (conflict == DxilSignatureAllocator::kNoConflict ||
      conflict == DxilSignatureAllocator::kInsufficientFreeComponents)
    conflict =
        allocator.DetectColConflict(&PE, E.GetStartRow(), E.GetStartCol());
  switch (conflict) {
  case DxilSignatureAllocator::kNoConflict:
    allocator.PlaceElement(&PE, E.GetStartRow(), E.GetStartCol());
    break;
  case DxilSignatureAllocator::kConflictsWithIndexed:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureIndexConflict,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kConflictsWithIndexedTessFactor:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureIndexConflict,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kConflictsWithInterpolationMode:
    ValCtx.EmitFormatError(ValidationRule::MetaInterpModeInOneRow,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kInsufficientFreeComponents:
    DXASSERT(false, "otherwise, conflict not translated");
    break;
  case DxilSignatureAllocator::kOverlapElement:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureOverlap,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kIllegalComponentOrder:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureIllegalComponentOrder,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kConflictFit:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureOutOfRange,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  case DxilSignatureAllocator::kConflictDataWidth:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureDataWidth,
                           {E.GetName(), std::to_string(E.GetStartRow()),
                            std::to_string(E.GetStartCol()),
                            std::to_string(E.GetRows()),
                            std::to_string(E.GetCols())});
    break;
  default:
    DXASSERT(
        false,
        "otherwise, unrecognized conflict type from DxilSignatureAllocator");
  }
}

static void ValidateSignature(ValidationContext &ValCtx, const DxilSignature &S,
                              EntryStatus &Status, unsigned maxScalars) {
  DxilSignatureAllocator allocator[DXIL::kNumOutputStreams] = {
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()}};
  unordered_set<unsigned> semanticUsageSet[DXIL::kNumOutputStreams];
  StringMap<unordered_set<unsigned>> semanticIndexMap[DXIL::kNumOutputStreams];
  unordered_set<unsigned> clipcullRowSet[DXIL::kNumOutputStreams];
  unsigned clipcullComponents[DXIL::kNumOutputStreams] = {0, 0, 0, 0};

  bool isOutput = S.IsOutput();
  unsigned TargetMask = 0;
  DXIL::SemanticKind DepthKind = DXIL::SemanticKind::Invalid;

  const InterpolationMode *prevBaryInterpMode = nullptr;
  unsigned numBarycentrics = 0;

  for (auto &E : S.GetElements()) {
    DXIL::SemanticKind semanticKind = E->GetSemantic()->GetKind();
    ValidateSignatureElement(*E, ValCtx);
    // Avoid OOB indexing on streamId.
    unsigned streamId = E->GetOutputStream();
    if (streamId >= DXIL::kNumOutputStreams || !isOutput ||
        !ValCtx.DxilMod.GetShaderModel()->IsGS()) {
      streamId = 0;
    }

    // Semantic index overlap check, keyed by name.
    std::string nameUpper(E->GetName());
    std::transform(nameUpper.begin(), nameUpper.end(), nameUpper.begin(),
                   ::toupper);
    unordered_set<unsigned> &semIdxSet = semanticIndexMap[streamId][nameUpper];
    for (unsigned semIdx : E->GetSemanticIndexVec()) {
      if (semIdxSet.count(semIdx) > 0) {
        ValCtx.EmitFormatError(ValidationRule::MetaNoSemanticOverlap,
                               {E->GetName(), std::to_string(semIdx)});
        return;
      } else
        semIdxSet.insert(semIdx);
    }

    // SV_Target has special rules
    if (semanticKind == DXIL::SemanticKind::Target) {
      // Validate target overlap
      if (E->GetStartRow() + E->GetRows() <= 8) {
        unsigned mask = ((1 << E->GetRows()) - 1) << E->GetStartRow();
        if (TargetMask & mask) {
          ValCtx.EmitFormatError(
              ValidationRule::MetaNoSemanticOverlap,
              {"SV_Target", std::to_string(E->GetStartRow())});
        }
        TargetMask = TargetMask | mask;
      }
      if (E->GetRows() > 1) {
        ValCtx.EmitSignatureError(E.get(), ValidationRule::SmNoPSOutputIdx);
      }
      continue;
    }

    if (E->GetSemantic()->IsInvalid())
      continue;

    // validate system value semantic rules
    switch (semanticKind) {
    case DXIL::SemanticKind::Arbitrary:
      break;
    case DXIL::SemanticKind::ClipDistance:
    case DXIL::SemanticKind::CullDistance:
      // Validate max 8 components across 2 rows (registers)
      for (unsigned rowIdx = 0; rowIdx < E->GetRows(); rowIdx++)
        clipcullRowSet[streamId].insert(E->GetStartRow() + rowIdx);
      if (clipcullRowSet[streamId].size() > 2) {
        ValCtx.EmitSignatureError(E.get(), ValidationRule::MetaClipCullMaxRows);
      }
      clipcullComponents[streamId] += E->GetCols();
      if (clipcullComponents[streamId] > 8) {
        ValCtx.EmitSignatureError(E.get(),
                                  ValidationRule::MetaClipCullMaxComponents);
      }
      break;
    case DXIL::SemanticKind::Depth:
    case DXIL::SemanticKind::DepthGreaterEqual:
    case DXIL::SemanticKind::DepthLessEqual:
      if (DepthKind != DXIL::SemanticKind::Invalid) {
        ValCtx.EmitSignatureError(E.get(),
                                  ValidationRule::SmPSMultipleDepthSemantic);
      }
      DepthKind = semanticKind;
      break;
    case DXIL::SemanticKind::Barycentrics: {
      // There can only be up to two SV_Barycentrics
      // with differeent perspective interpolation modes.
      if (numBarycentrics++ > 1) {
        ValCtx.EmitSignatureError(
            E.get(), ValidationRule::MetaBarycentricsTwoPerspectives);
        break;
      }
      const InterpolationMode *mode = E->GetInterpolationMode();
      if (prevBaryInterpMode) {
        if ((mode->IsAnyNoPerspective() &&
             prevBaryInterpMode->IsAnyNoPerspective()) ||
            (!mode->IsAnyNoPerspective() &&
             !prevBaryInterpMode->IsAnyNoPerspective())) {
          ValCtx.EmitSignatureError(
              E.get(), ValidationRule::MetaBarycentricsTwoPerspectives);
        }
      }
      prevBaryInterpMode = mode;
      break;
    }
    default:
      if (semanticUsageSet[streamId].count(
              static_cast<unsigned>(semanticKind)) > 0) {
        ValCtx.EmitFormatError(ValidationRule::MetaDuplicateSysValue,
                               {E->GetSemantic()->GetName()});
      }
      semanticUsageSet[streamId].insert(static_cast<unsigned>(semanticKind));
      break;
    }

    // Packed element overlap check.
    ValidateSignatureOverlap(*E.get(), maxScalars, allocator[streamId], ValCtx);

    if (isOutput && semanticKind == DXIL::SemanticKind::Position) {
      Status.hasOutputPosition[E->GetOutputStream()] = true;
    }
  }

  if (Status.hasViewID && S.IsInput() &&
      ValCtx.DxilMod.GetShaderModel()->GetKind() == DXIL::ShaderKind::Pixel) {
    // Ensure sufficient space for ViewID:
    DxilSignatureAllocator::DummyElement viewID;
    viewID.rows = 1;
    viewID.cols = 1;
    viewID.kind = DXIL::SemanticKind::Arbitrary;
    viewID.interpolation = DXIL::InterpolationMode::Constant;
    viewID.interpretation = DXIL::SemanticInterpretationKind::SGV;
    allocator[0].PackNext(&viewID, 0, 32);
    if (!viewID.IsAllocated()) {
      ValCtx.EmitError(ValidationRule::SmViewIDNeedsSlot);
    }
  }
}

static void ValidateNoInterpModeSignature(ValidationContext &ValCtx,
                                          const DxilSignature &S) {
  for (auto &E : S.GetElements()) {
    if (!E->GetInterpolationMode()->IsUndefined()) {
      ValCtx.EmitSignatureError(E.get(), ValidationRule::SmNoInterpMode);
    }
  }
}

static void ValidateConstantInterpModeSignature(ValidationContext &ValCtx,
                                                const DxilSignature &S) {
  for (auto &E : S.GetElements()) {
    if (!E->GetInterpolationMode()->IsConstant()) {
      ValCtx.EmitSignatureError(E.get(), ValidationRule::SmConstantInterpMode);
    }
  }
}

static void ValidateEntrySignatures(ValidationContext &ValCtx,
                                    const DxilEntryProps &entryProps,
                                    EntryStatus &Status, Function &F) {
  const DxilFunctionProps &props = entryProps.props;
  const DxilEntrySignature &S = entryProps.sig;

  if (props.IsRay()) {
    // No signatures allowed
    if (!S.InputSignature.GetElements().empty() ||
        !S.OutputSignature.GetElements().empty() ||
        !S.PatchConstOrPrimSignature.GetElements().empty()) {
      ValCtx.EmitFnFormatError(&F, ValidationRule::SmRayShaderSignatures,
                               {F.getName()});
    }

    // Validate payload/attribute/params sizes
    unsigned payloadSize = 0;
    unsigned attrSize = 0;
    auto itPayload = F.arg_begin();
    auto itAttr = itPayload;
    if (itAttr != F.arg_end())
      itAttr++;
    DataLayout DL(F.getParent());
    switch (props.shaderKind) {
    case DXIL::ShaderKind::AnyHit:
    case DXIL::ShaderKind::ClosestHit:
      if (itAttr != F.arg_end()) {
        Type *Ty = itAttr->getType();
        if (Ty->isPointerTy())
          Ty = Ty->getPointerElementType();
        attrSize =
            (unsigned)std::min(DL.getTypeAllocSize(Ty), (uint64_t)UINT_MAX);
      }
      LLVM_FALLTHROUGH;
    case DXIL::ShaderKind::Miss:
    case DXIL::ShaderKind::Callable:
      if (itPayload != F.arg_end()) {
        Type *Ty = itPayload->getType();
        if (Ty->isPointerTy())
          Ty = Ty->getPointerElementType();
        payloadSize =
            (unsigned)std::min(DL.getTypeAllocSize(Ty), (uint64_t)UINT_MAX);
      }
      break;
    }
    if (props.ShaderProps.Ray.payloadSizeInBytes < payloadSize) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmRayShaderPayloadSize,
          {F.getName(), props.IsCallable() ? "params" : "payload"});
    }
    if (props.ShaderProps.Ray.attributeSizeInBytes < attrSize) {
      ValCtx.EmitFnFormatError(&F, ValidationRule::SmRayShaderPayloadSize,
                               {F.getName(), "attribute"});
    }
    return;
  }

  bool isPS = props.IsPS();
  bool isVS = props.IsVS();
  bool isGS = props.IsGS();
  bool isCS = props.IsCS();
  bool isMS = props.IsMS();

  if (isPS) {
    // PS output no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.OutputSignature);
  } else if (isVS) {
    // VS input no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.InputSignature);
  }

  if (isMS) {
    // primitive output constant interp mode.
    ValidateConstantInterpModeSignature(ValCtx, S.PatchConstOrPrimSignature);
  } else {
    // patch constant no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.PatchConstOrPrimSignature);
  }

  unsigned maxInputScalars = DXIL::kMaxInputTotalScalars;
  unsigned maxOutputScalars = 0;
  unsigned maxPatchConstantScalars = 0;

  switch (props.shaderKind) {
  case DXIL::ShaderKind::Compute:
    break;
  case DXIL::ShaderKind::Vertex:
  case DXIL::ShaderKind::Geometry:
  case DXIL::ShaderKind::Pixel:
    maxOutputScalars = DXIL::kMaxOutputTotalScalars;
    break;
  case DXIL::ShaderKind::Hull:
  case DXIL::ShaderKind::Domain:
    maxOutputScalars = DXIL::kMaxOutputTotalScalars;
    maxPatchConstantScalars = DXIL::kMaxHSOutputPatchConstantTotalScalars;
    break;
  case DXIL::ShaderKind::Mesh:
    maxOutputScalars = DXIL::kMaxOutputTotalScalars;
    maxPatchConstantScalars = DXIL::kMaxOutputTotalScalars;
    break;
  case DXIL::ShaderKind::Amplification:
  default:
    break;
  }

  ValidateSignature(ValCtx, S.InputSignature, Status, maxInputScalars);
  ValidateSignature(ValCtx, S.OutputSignature, Status, maxOutputScalars);
  ValidateSignature(ValCtx, S.PatchConstOrPrimSignature, Status,
                    maxPatchConstantScalars);

  if (isPS) {
    // Gather execution information.
    hlsl::PSExecutionInfo PSExec;
    DxilSignatureElement *PosInterpSE = nullptr;
    for (auto &E : S.InputSignature.GetElements()) {
      if (E->GetKind() == DXIL::SemanticKind::SampleIndex) {
        PSExec.SuperSampling = true;
        continue;
      }

      const InterpolationMode *IM = E->GetInterpolationMode();
      if (IM->IsLinearSample() || IM->IsLinearNoperspectiveSample()) {
        PSExec.SuperSampling = true;
      }
      if (E->GetKind() == DXIL::SemanticKind::Position) {
        PSExec.PositionInterpolationMode = IM;
        PosInterpSE = E.get();
      }
    }

    for (auto &E : S.OutputSignature.GetElements()) {
      if (E->IsAnyDepth()) {
        PSExec.OutputDepthKind = E->GetKind();
        break;
      }
    }

    if (!PSExec.SuperSampling &&
        PSExec.OutputDepthKind != DXIL::SemanticKind::Invalid &&
        PSExec.OutputDepthKind != DXIL::SemanticKind::Depth) {
      if (PSExec.PositionInterpolationMode != nullptr) {
        if (!PSExec.PositionInterpolationMode->IsUndefined() &&
            !PSExec.PositionInterpolationMode
                 ->IsLinearNoperspectiveCentroid() &&
            !PSExec.PositionInterpolationMode->IsLinearNoperspectiveSample()) {
          ValCtx.EmitFnFormatError(&F, ValidationRule::SmPSConsistentInterp,
                                   {PosInterpSE->GetName()});
        }
      }
    }

    // Validate PS output semantic.
    const DxilSignature &outputSig = S.OutputSignature;
    for (auto &SE : outputSig.GetElements()) {
      Semantic::Kind semanticKind = SE->GetSemantic()->GetKind();
      switch (semanticKind) {
      case Semantic::Kind::Target:
      case Semantic::Kind::Coverage:
      case Semantic::Kind::Depth:
      case Semantic::Kind::DepthGreaterEqual:
      case Semantic::Kind::DepthLessEqual:
      case Semantic::Kind::StencilRef:
        break;
      default: {
        ValCtx.EmitFnFormatError(&F, ValidationRule::SmPSOutputSemantic,
                                 {SE->GetName()});
      } break;
      }
    }
  }

  if (isGS) {
    unsigned maxVertexCount = props.ShaderProps.GS.maxVertexCount;
    unsigned outputScalarCount = 0;
    const DxilSignature &outSig = S.OutputSignature;
    for (auto &SE : outSig.GetElements()) {
      outputScalarCount += SE->GetRows() * SE->GetCols();
    }
    unsigned totalOutputScalars = maxVertexCount * outputScalarCount;
    if (totalOutputScalars > DXIL::kMaxGSOutputTotalScalars) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmGSTotalOutputVertexDataRange,
          {std::to_string(maxVertexCount), std::to_string(outputScalarCount),
           std::to_string(totalOutputScalars),
           std::to_string(DXIL::kMaxGSOutputTotalScalars)});
    }
  }

  if (isCS) {
    if (!S.InputSignature.GetElements().empty() ||
        !S.OutputSignature.GetElements().empty() ||
        !S.PatchConstOrPrimSignature.GetElements().empty()) {
      ValCtx.EmitFnError(&F, ValidationRule::SmCSNoSignatures);
    }
  }

  if (isMS) {
    unsigned VertexSignatureRows = S.OutputSignature.GetRowCount();
    if (VertexSignatureRows > DXIL::kMaxMSVSigRows) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshVSigRowCount,
          {F.getName(), std::to_string(DXIL::kMaxMSVSigRows)});
    }
    unsigned PrimitiveSignatureRows = S.PatchConstOrPrimSignature.GetRowCount();
    if (PrimitiveSignatureRows > DXIL::kMaxMSPSigRows) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshPSigRowCount,
          {F.getName(), std::to_string(DXIL::kMaxMSPSigRows)});
    }
    if (VertexSignatureRows + PrimitiveSignatureRows >
        DXIL::kMaxMSTotalSigRows) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshTotalSigRowCount,
          {F.getName(), std::to_string(DXIL::kMaxMSTotalSigRows)});
    }

    const unsigned kScalarSizeForMSAttributes = 4;
#define ALIGN32(n) (((n) + 31) & ~31)
    unsigned maxAlign32VertexCount =
        ALIGN32(props.ShaderProps.MS.maxVertexCount);
    unsigned maxAlign32PrimitiveCount =
        ALIGN32(props.ShaderProps.MS.maxPrimitiveCount);
    unsigned totalOutputScalars = 0;
    for (auto &SE : S.OutputSignature.GetElements()) {
      totalOutputScalars +=
          SE->GetRows() * SE->GetCols() * maxAlign32VertexCount;
    }
    for (auto &SE : S.PatchConstOrPrimSignature.GetElements()) {
      totalOutputScalars +=
          SE->GetRows() * SE->GetCols() * maxAlign32PrimitiveCount;
    }

    if (totalOutputScalars * kScalarSizeForMSAttributes >
        DXIL::kMaxMSOutputTotalBytes) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshShaderOutputSize,
          {F.getName(), std::to_string(DXIL::kMaxMSOutputTotalBytes)});
    }

    unsigned totalInputOutputBytes =
        totalOutputScalars * kScalarSizeForMSAttributes +
        props.ShaderProps.MS.payloadSizeInBytes;
    if (totalInputOutputBytes > DXIL::kMaxMSInputOutputTotalBytes) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshShaderInOutSize,
          {F.getName(), std::to_string(DXIL::kMaxMSInputOutputTotalBytes)});
    }
  }
}

static void ValidateEntrySignatures(ValidationContext &ValCtx) {
  DxilModule &DM = ValCtx.DxilMod;
  if (ValCtx.isLibProfile) {
    for (Function &F : DM.GetModule()->functions()) {
      if (DM.HasDxilEntryProps(&F)) {
        DxilEntryProps &entryProps = DM.GetDxilEntryProps(&F);
        EntryStatus &Status = ValCtx.GetEntryStatus(&F);
        ValidateEntrySignatures(ValCtx, entryProps, Status, F);
      }
    }
  } else {
    Function *Entry = DM.GetEntryFunction();
    if (!DM.HasDxilEntryProps(Entry)) {
      // must have props.
      ValCtx.EmitFnError(Entry, ValidationRule::MetaNoEntryPropsForEntry);
      return;
    }
    EntryStatus &Status = ValCtx.GetEntryStatus(Entry);
    DxilEntryProps &entryProps = DM.GetDxilEntryProps(Entry);
    ValidateEntrySignatures(ValCtx, entryProps, Status, *Entry);
  }
}

// CompatibilityChecker is used to identify incompatibilities in an entry
// function and any functions called by that entry function.
struct CompatibilityChecker {
  ValidationContext &ValCtx;
  Function *EntryFn;
  const DxilFunctionProps &props;
  DXIL::ShaderKind shaderKind;

  // These masks identify the potential conflict flags based on the entry
  // function's shader kind and properties when either UsesDerivatives or
  // RequiresGroup flags are set in ShaderCompatInfo.
  uint32_t maskForDeriv = 0;
  uint32_t maskForGroup = 0;

  enum class ConflictKind : uint32_t {
    Stage,
    ShaderModel,
    DerivLaunch,
    DerivThreadGroupDim,
    DerivInComputeShaderModel,
    RequiresGroup,
  };
  enum class ConflictFlags : uint32_t {
    Stage = 1 << (uint32_t)ConflictKind::Stage,
    ShaderModel = 1 << (uint32_t)ConflictKind::ShaderModel,
    DerivLaunch = 1 << (uint32_t)ConflictKind::DerivLaunch,
    DerivThreadGroupDim = 1 << (uint32_t)ConflictKind::DerivThreadGroupDim,
    DerivInComputeShaderModel =
        1 << (uint32_t)ConflictKind::DerivInComputeShaderModel,
    RequiresGroup = 1 << (uint32_t)ConflictKind::RequiresGroup,
  };

  CompatibilityChecker(ValidationContext &ValCtx, Function *EntryFn)
      : ValCtx(ValCtx), EntryFn(EntryFn),
        props(ValCtx.DxilMod.GetDxilEntryProps(EntryFn).props),
        shaderKind(props.shaderKind) {

    // Precompute potential incompatibilities based on shader stage, shader kind
    // and entry attributes. These will turn into full conflicts if the entry
    // point's shader flags indicate that they use relevant features.
    if (!ValCtx.DxilMod.GetShaderModel()->IsSM66Plus() &&
        (shaderKind == DXIL::ShaderKind::Mesh ||
         shaderKind == DXIL::ShaderKind::Amplification ||
         shaderKind == DXIL::ShaderKind::Compute)) {
      maskForDeriv |=
          static_cast<uint32_t>(ConflictFlags::DerivInComputeShaderModel);
    } else if (shaderKind == DXIL::ShaderKind::Node) {
      // Only broadcasting launch supports derivatives.
      if (props.Node.LaunchType != DXIL::NodeLaunchType::Broadcasting)
        maskForDeriv |= static_cast<uint32_t>(ConflictFlags::DerivLaunch);
      // Thread launch node has no group.
      if (props.Node.LaunchType == DXIL::NodeLaunchType::Thread)
        maskForGroup |= static_cast<uint32_t>(ConflictFlags::RequiresGroup);
    }

    if (shaderKind == DXIL::ShaderKind::Mesh ||
        shaderKind == DXIL::ShaderKind::Amplification ||
        shaderKind == DXIL::ShaderKind::Compute ||
        shaderKind == DXIL::ShaderKind::Node) {
      // All compute-like stages
      // Thread dimensions must be either 1D and X is multiple of 4, or 2D
      // and X and Y must be multiples of 2.
      if (props.numThreads[1] == 1 && props.numThreads[2] == 1) {
        if ((props.numThreads[0] & 0x3) != 0)
          maskForDeriv |=
              static_cast<uint32_t>(ConflictFlags::DerivThreadGroupDim);
      } else if ((props.numThreads[0] & 0x1) || (props.numThreads[1] & 0x1))
        maskForDeriv |=
            static_cast<uint32_t>(ConflictFlags::DerivThreadGroupDim);
    } else {
      // other stages have no group
      maskForGroup |= static_cast<uint32_t>(ConflictFlags::RequiresGroup);
    }
  }

  uint32_t
  IdentifyConflict(const DxilModule::ShaderCompatInfo &compatInfo) const {
    uint32_t conflictMask = 0;

    // Compatibility check said this shader kind is not compatible.
    if (0 == ((1 << (uint32_t)shaderKind) & compatInfo.mask))
      conflictMask |= (uint32_t)ConflictFlags::Stage;

    // Compatibility check said this shader model is not compatible.
    if (DXIL::CompareVersions(ValCtx.DxilMod.GetShaderModel()->GetMajor(),
                              ValCtx.DxilMod.GetShaderModel()->GetMinor(),
                              compatInfo.minMajor, compatInfo.minMinor) < 0)
      conflictMask |= (uint32_t)ConflictFlags::ShaderModel;

    if (compatInfo.shaderFlags.GetUsesDerivatives())
      conflictMask |= maskForDeriv;

    if (compatInfo.shaderFlags.GetRequiresGroup())
      conflictMask |= maskForGroup;

    return conflictMask;
  }

  void Diagnose(Function *F, uint32_t conflictMask, ConflictKind conflict,
                ValidationRule rule, ArrayRef<StringRef> args = {}) {
    if (conflictMask & (1 << (unsigned)conflict))
      ValCtx.EmitFnFormatError(F, rule, args);
  }

  void DiagnoseConflicts(Function *F, uint32_t conflictMask) {
    // Emit a diagnostic indicating that either the entry function or a function
    // called by the entry function contains a disallowed operation.
    if (F == EntryFn)
      ValCtx.EmitFnError(EntryFn, ValidationRule::SmIncompatibleOperation);
    else
      ValCtx.EmitFnError(EntryFn, ValidationRule::SmIncompatibleCallInEntry);

    // Emit diagnostics for each conflict found in this function.
    Diagnose(F, conflictMask, ConflictKind::Stage,
             ValidationRule::SmIncompatibleStage,
             {ShaderModel::GetKindName(props.shaderKind)});
    Diagnose(F, conflictMask, ConflictKind::ShaderModel,
             ValidationRule::SmIncompatibleShaderModel);
    Diagnose(F, conflictMask, ConflictKind::DerivLaunch,
             ValidationRule::SmIncompatibleDerivLaunch,
             {GetLaunchTypeStr(props.Node.LaunchType)});
    Diagnose(F, conflictMask, ConflictKind::DerivThreadGroupDim,
             ValidationRule::SmIncompatibleThreadGroupDim,
             {std::to_string(props.numThreads[0]),
              std::to_string(props.numThreads[1]),
              std::to_string(props.numThreads[2])});
    Diagnose(F, conflictMask, ConflictKind::DerivInComputeShaderModel,
             ValidationRule::SmIncompatibleDerivInComputeShaderModel);
    Diagnose(F, conflictMask, ConflictKind::RequiresGroup,
             ValidationRule::SmIncompatibleRequiresGroup);
  }

  // Visit function and all functions called by it.
  // Emit diagnostics for incompatibilities found in a function when no
  // functions called by that function introduced the conflict.
  // In those cases, the called functions themselves will emit the diagnostic.
  // Return conflict mask for this function.
  uint32_t Visit(Function *F, uint32_t &remainingMask,
                 llvm::SmallPtrSet<Function *, 8> &visited, CallGraph &CG) {
    // Recursive check looks for where a conflict is found and not present
    // in functions called by the current function.
    // - When a source is found, emit diagnostics and clear the conflict
    // flags introduced by this function from the working mask so we don't
    // report this conflict again.
    // - When the remainingMask is 0, we are done.

    if (remainingMask == 0)
      return 0; // Nothing left to search for.
    if (!visited.insert(F).second)
      return 0; // Already visited.

    const DxilModule::ShaderCompatInfo *compatInfo =
        ValCtx.DxilMod.GetCompatInfoForFunction(F);
    DXASSERT(compatInfo, "otherwise, compat info not computed in module");
    if (!compatInfo)
      return 0;
    uint32_t maskForThisFunction = IdentifyConflict(*compatInfo);

    uint32_t maskForCalls = 0;
    if (CallGraphNode *CGNode = CG[F]) {
      for (auto &Call : *CGNode) {
        Function *called = Call.second->getFunction();
        if (called->isDeclaration())
          continue;
        maskForCalls |= Visit(called, remainingMask, visited, CG);
        if (remainingMask == 0)
          return 0; // Nothing left to search for.
      }
    }

    // Mask of incompatibilities introduced by this function.
    uint32_t conflictsIntroduced =
        remainingMask & maskForThisFunction & ~maskForCalls;
    if (conflictsIntroduced) {
      // This function introduces at least one conflict.
      DiagnoseConflicts(F, conflictsIntroduced);
      // Mask off diagnosed incompatibilities.
      remainingMask &= ~conflictsIntroduced;
    }
    return maskForThisFunction;
  }

  void FindIncompatibleCall(const DxilModule::ShaderCompatInfo &compatInfo) {
    uint32_t conflictMask = IdentifyConflict(compatInfo);
    if (conflictMask == 0)
      return;

    CallGraph &CG = ValCtx.GetCallGraph();
    llvm::SmallPtrSet<Function *, 8> visited;
    Visit(EntryFn, conflictMask, visited, CG);
  }
};

static void ValidateEntryCompatibility(ValidationContext &ValCtx) {
  // Make sure functions called from each entry are compatible with that entry.
  DxilModule &DM = ValCtx.DxilMod;
  for (Function &F : DM.GetModule()->functions()) {
    if (DM.HasDxilEntryProps(&F)) {
      const DxilModule::ShaderCompatInfo *compatInfo =
          DM.GetCompatInfoForFunction(&F);
      DXASSERT(compatInfo, "otherwise, compat info not computed in module");
      if (!compatInfo)
        continue;

      CompatibilityChecker checker(ValCtx, &F);
      checker.FindIncompatibleCall(*compatInfo);
    }
  }
}

static void CheckPatchConstantSemantic(ValidationContext &ValCtx,
                                       const DxilEntryProps &EntryProps,
                                       EntryStatus &Status, Function *F) {
  const DxilFunctionProps &props = EntryProps.props;
  bool isHS = props.IsHS();

  DXIL::TessellatorDomain domain =
      isHS ? props.ShaderProps.HS.domain : props.ShaderProps.DS.domain;

  const DxilSignature &patchConstantSig =
      EntryProps.sig.PatchConstOrPrimSignature;

  const unsigned kQuadEdgeSize = 4;
  const unsigned kQuadInsideSize = 2;
  const unsigned kQuadDomainLocSize = 2;

  const unsigned kTriEdgeSize = 3;
  const unsigned kTriInsideSize = 1;
  const unsigned kTriDomainLocSize = 3;

  const unsigned kIsolineEdgeSize = 2;
  const unsigned kIsolineInsideSize = 0;
  const unsigned kIsolineDomainLocSize = 3;

  const char *domainName = "";

  DXIL::SemanticKind kEdgeSemantic = DXIL::SemanticKind::TessFactor;
  unsigned edgeSize = 0;

  DXIL::SemanticKind kInsideSemantic = DXIL::SemanticKind::InsideTessFactor;
  unsigned insideSize = 0;

  Status.domainLocSize = 0;

  switch (domain) {
  case DXIL::TessellatorDomain::IsoLine:
    domainName = "IsoLine";
    edgeSize = kIsolineEdgeSize;
    insideSize = kIsolineInsideSize;
    Status.domainLocSize = kIsolineDomainLocSize;
    break;
  case DXIL::TessellatorDomain::Tri:
    domainName = "Tri";
    edgeSize = kTriEdgeSize;
    insideSize = kTriInsideSize;
    Status.domainLocSize = kTriDomainLocSize;
    break;
  case DXIL::TessellatorDomain::Quad:
    domainName = "Quad";
    edgeSize = kQuadEdgeSize;
    insideSize = kQuadInsideSize;
    Status.domainLocSize = kQuadDomainLocSize;
    break;
  default:
    // Don't bother with other tests if domain is invalid
    return;
  }

  bool bFoundEdgeSemantic = false;
  bool bFoundInsideSemantic = false;
  for (auto &SE : patchConstantSig.GetElements()) {
    Semantic::Kind kind = SE->GetSemantic()->GetKind();
    if (kind == kEdgeSemantic) {
      bFoundEdgeSemantic = true;
      if (SE->GetRows() != edgeSize || SE->GetCols() > 1) {
        ValCtx.EmitFnFormatError(F, ValidationRule::SmTessFactorSizeMatchDomain,
                                 {std::to_string(SE->GetRows()),
                                  std::to_string(SE->GetCols()), domainName,
                                  std::to_string(edgeSize)});
      }
    } else if (kind == kInsideSemantic) {
      bFoundInsideSemantic = true;
      if (SE->GetRows() != insideSize || SE->GetCols() > 1) {
        ValCtx.EmitFnFormatError(
            F, ValidationRule::SmInsideTessFactorSizeMatchDomain,
            {std::to_string(SE->GetRows()), std::to_string(SE->GetCols()),
             domainName, std::to_string(insideSize)});
      }
    }
  }

  if (isHS) {
    if (!bFoundEdgeSemantic) {
      ValCtx.EmitFnError(F, ValidationRule::SmTessFactorForDomain);
    }
    if (!bFoundInsideSemantic && domain != DXIL::TessellatorDomain::IsoLine) {
      ValCtx.EmitFnError(F, ValidationRule::SmTessFactorForDomain);
    }
  }
}

static void ValidatePassThruHS(ValidationContext &ValCtx,
                               const DxilEntryProps &entryProps, Function *F) {
  // Check pass thru HS.
  if (F->isDeclaration()) {
    const auto &props = entryProps.props;
    if (props.IsHS()) {
      const auto &HS = props.ShaderProps.HS;
      if (HS.inputControlPoints < HS.outputControlPoints) {
        ValCtx.EmitFnError(
            F, ValidationRule::SmHullPassThruControlPointCountMatch);
      }

      // Check declared control point outputs storage amounts are ok to pass
      // through (less output storage than input for control points).
      const DxilSignature &outSig = entryProps.sig.OutputSignature;
      unsigned totalOutputCPScalars = 0;
      for (auto &SE : outSig.GetElements()) {
        totalOutputCPScalars += SE->GetRows() * SE->GetCols();
      }
      if (totalOutputCPScalars * HS.outputControlPoints >
          DXIL::kMaxHSOutputControlPointsTotalScalars) {
        ValCtx.EmitFnError(F,
                           ValidationRule::SmOutputControlPointsTotalScalars);
        // TODO: add number at end. need format fn error?
      }
    } else {
      ValCtx.EmitFnError(F, ValidationRule::MetaEntryFunction);
    }
  }
}

// validate wave size (currently allowed only on CS and node shaders but might
// be supported on other shader types in the future)
static void ValidateWaveSize(ValidationContext &ValCtx,
                             const DxilEntryProps &entryProps, Function *F) {
  const DxilFunctionProps &props = entryProps.props;
  const hlsl::DxilWaveSize &waveSize = props.WaveSize;

  switch (waveSize.Validate()) {
  case hlsl::DxilWaveSize::ValidationResult::Success:
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidMin:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Min", std::to_string(waveSize.Min),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidMax:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Max", std::to_string(waveSize.Max),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidPreferred:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Preferred", std::to_string(waveSize.Preferred),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxOrPreferredWhenUndefined:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeAllZeroWhenUndefined,
        {std::to_string(waveSize.Max), std::to_string(waveSize.Preferred)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxEqualsMin:
    // This case is allowed because users may disable the ErrorDefault warning.
    break;
  case hlsl::DxilWaveSize::ValidationResult::PreferredWhenNoRange:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeMaxAndPreferredZeroWhenNoRange,
        {std::to_string(waveSize.Max), std::to_string(waveSize.Preferred)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxLessThanMin:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeMaxGreaterThanMin,
        {std::to_string(waveSize.Max), std::to_string(waveSize.Min)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::PreferredOutOfRange:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizePreferredInRange,
                             {std::to_string(waveSize.Preferred),
                              std::to_string(waveSize.Min),
                              std::to_string(waveSize.Max)});
    break;
  }

  // Check shader model and kind.
  if (waveSize.IsDefined()) {
    if (!props.IsCS() && !props.IsNode()) {
      ValCtx.EmitFnError(F, ValidationRule::SmWaveSizeOnComputeOrNode);
    }
  }
}

static void ValidateEntryProps(ValidationContext &ValCtx,
                               const DxilEntryProps &entryProps,
                               EntryStatus &Status, Function *F) {
  const DxilFunctionProps &props = entryProps.props;
  DXIL::ShaderKind ShaderType = props.shaderKind;

  ValidateWaveSize(ValCtx, entryProps, F);

  if (ShaderType == DXIL::ShaderKind::Compute || props.IsNode()) {
    unsigned x = props.numThreads[0];
    unsigned y = props.numThreads[1];
    unsigned z = props.numThreads[2];

    unsigned threadsInGroup = x * y * z;

    if ((x < DXIL::kMinCSThreadGroupX) || (x > DXIL::kMaxCSThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(x),
                                std::to_string(DXIL::kMinCSThreadGroupX),
                                std::to_string(DXIL::kMaxCSThreadGroupX)});
    }
    if ((y < DXIL::kMinCSThreadGroupY) || (y > DXIL::kMaxCSThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(y),
                                std::to_string(DXIL::kMinCSThreadGroupY),
                                std::to_string(DXIL::kMaxCSThreadGroupY)});
    }
    if ((z < DXIL::kMinCSThreadGroupZ) || (z > DXIL::kMaxCSThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(z),
                                std::to_string(DXIL::kMinCSThreadGroupZ),
                                std::to_string(DXIL::kMaxCSThreadGroupZ)});
    }

    if (threadsInGroup > DXIL::kMaxCSThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(threadsInGroup),
                                std::to_string(DXIL::kMaxCSThreadsPerGroup)});
    }

    // type of threadID, thread group ID take care by DXIL operation overload
    // check.
  } else if (ShaderType == DXIL::ShaderKind::Mesh) {
    const auto &MS = props.ShaderProps.MS;
    unsigned x = props.numThreads[0];
    unsigned y = props.numThreads[1];
    unsigned z = props.numThreads[2];

    unsigned threadsInGroup = x * y * z;

    if ((x < DXIL::kMinMSASThreadGroupX) || (x > DXIL::kMaxMSASThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(x),
                                std::to_string(DXIL::kMinMSASThreadGroupX),
                                std::to_string(DXIL::kMaxMSASThreadGroupX)});
    }
    if ((y < DXIL::kMinMSASThreadGroupY) || (y > DXIL::kMaxMSASThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(y),
                                std::to_string(DXIL::kMinMSASThreadGroupY),
                                std::to_string(DXIL::kMaxMSASThreadGroupY)});
    }
    if ((z < DXIL::kMinMSASThreadGroupZ) || (z > DXIL::kMaxMSASThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(z),
                                std::to_string(DXIL::kMinMSASThreadGroupZ),
                                std::to_string(DXIL::kMaxMSASThreadGroupZ)});
    }

    if (threadsInGroup > DXIL::kMaxMSASThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(threadsInGroup),
                                std::to_string(DXIL::kMaxMSASThreadsPerGroup)});
    }

    // type of threadID, thread group ID take care by DXIL operation overload
    // check.

    unsigned maxVertexCount = MS.maxVertexCount;
    if (maxVertexCount > DXIL::kMaxMSOutputVertexCount) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMeshShaderMaxVertexCount,
                               {std::to_string(DXIL::kMaxMSOutputVertexCount),
                                std::to_string(maxVertexCount)});
    }

    unsigned maxPrimitiveCount = MS.maxPrimitiveCount;
    if (maxPrimitiveCount > DXIL::kMaxMSOutputPrimitiveCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderMaxPrimitiveCount,
          {std::to_string(DXIL::kMaxMSOutputPrimitiveCount),
           std::to_string(maxPrimitiveCount)});
    }
  } else if (ShaderType == DXIL::ShaderKind::Amplification) {
    unsigned x = props.numThreads[0];
    unsigned y = props.numThreads[1];
    unsigned z = props.numThreads[2];

    unsigned threadsInGroup = x * y * z;

    if ((x < DXIL::kMinMSASThreadGroupX) || (x > DXIL::kMaxMSASThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(x),
                                std::to_string(DXIL::kMinMSASThreadGroupX),
                                std::to_string(DXIL::kMaxMSASThreadGroupX)});
    }
    if ((y < DXIL::kMinMSASThreadGroupY) || (y > DXIL::kMaxMSASThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(y),
                                std::to_string(DXIL::kMinMSASThreadGroupY),
                                std::to_string(DXIL::kMaxMSASThreadGroupY)});
    }
    if ((z < DXIL::kMinMSASThreadGroupZ) || (z > DXIL::kMaxMSASThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(z),
                                std::to_string(DXIL::kMinMSASThreadGroupZ),
                                std::to_string(DXIL::kMaxMSASThreadGroupZ)});
    }

    if (threadsInGroup > DXIL::kMaxMSASThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(threadsInGroup),
                                std::to_string(DXIL::kMaxMSASThreadsPerGroup)});
    }

    // type of threadID, thread group ID take care by DXIL operation overload
    // check.
  } else if (ShaderType == DXIL::ShaderKind::Domain) {
    const auto &DS = props.ShaderProps.DS;
    DXIL::TessellatorDomain domain = DS.domain;
    if (domain >= DXIL::TessellatorDomain::LastEntry)
      domain = DXIL::TessellatorDomain::Undefined;
    unsigned inputControlPointCount = DS.inputControlPoints;

    if (inputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmDSInputControlPointCountRange,
          {std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(inputControlPointCount)});
    }
    if (domain == DXIL::TessellatorDomain::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmValidDomain);
    }
    CheckPatchConstantSemantic(ValCtx, entryProps, Status, F);
  } else if (ShaderType == DXIL::ShaderKind::Hull) {
    const auto &HS = props.ShaderProps.HS;
    DXIL::TessellatorDomain domain = HS.domain;
    if (domain >= DXIL::TessellatorDomain::LastEntry)
      domain = DXIL::TessellatorDomain::Undefined;
    unsigned inputControlPointCount = HS.inputControlPoints;
    if (inputControlPointCount == 0) {
      const DxilSignature &inputSig = entryProps.sig.InputSignature;
      if (!inputSig.GetElements().empty()) {
        ValCtx.EmitFnError(F,
                           ValidationRule::SmZeroHSInputControlPointWithInput);
      }
    } else if (inputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmHSInputControlPointCountRange,
          {std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(inputControlPointCount)});
    }

    unsigned outputControlPointCount = HS.outputControlPoints;
    if (outputControlPointCount < DXIL::kMinIAPatchControlPointCount ||
        outputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmOutputControlPointCountRange,
          {std::to_string(DXIL::kMinIAPatchControlPointCount),
           std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(outputControlPointCount)});
    }
    if (domain == DXIL::TessellatorDomain::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmValidDomain);
    }
    DXIL::TessellatorPartitioning partition = HS.partition;
    if (partition == DXIL::TessellatorPartitioning::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::MetaTessellatorPartition);
    }

    DXIL::TessellatorOutputPrimitive tessOutputPrimitive = HS.outputPrimitive;
    if (tessOutputPrimitive == DXIL::TessellatorOutputPrimitive::Undefined ||
        tessOutputPrimitive == DXIL::TessellatorOutputPrimitive::LastEntry) {
      ValCtx.EmitFnError(F, ValidationRule::MetaTessellatorOutputPrimitive);
    }

    float maxTessFactor = HS.maxTessFactor;
    if (maxTessFactor < DXIL::kHSMaxTessFactorLowerBound ||
        maxTessFactor > DXIL::kHSMaxTessFactorUpperBound) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::MetaMaxTessFactor,
          {std::to_string(DXIL::kHSMaxTessFactorLowerBound),
           std::to_string(DXIL::kHSMaxTessFactorUpperBound),
           std::to_string(maxTessFactor)});
    }
    // Domain and OutPrimivtive match.
    switch (domain) {
    case DXIL::TessellatorDomain::IsoLine:
      switch (tessOutputPrimitive) {
      case DXIL::TessellatorOutputPrimitive::TriangleCW:
      case DXIL::TessellatorOutputPrimitive::TriangleCCW:
        ValCtx.EmitFnError(F, ValidationRule::SmIsoLineOutputPrimitiveMismatch);
        break;
      default:
        break;
      }
      break;
    case DXIL::TessellatorDomain::Tri:
      switch (tessOutputPrimitive) {
      case DXIL::TessellatorOutputPrimitive::Line:
        ValCtx.EmitFnError(F, ValidationRule::SmTriOutputPrimitiveMismatch);
        break;
      default:
        break;
      }
      break;
    case DXIL::TessellatorDomain::Quad:
      switch (tessOutputPrimitive) {
      case DXIL::TessellatorOutputPrimitive::Line:
        ValCtx.EmitFnError(F, ValidationRule::SmTriOutputPrimitiveMismatch);
        break;
      default:
        break;
      }
      break;
    default:
      ValCtx.EmitFnError(F, ValidationRule::SmValidDomain);
      break;
    }

    CheckPatchConstantSemantic(ValCtx, entryProps, Status, F);
  } else if (ShaderType == DXIL::ShaderKind::Geometry) {
    const auto &GS = props.ShaderProps.GS;
    unsigned maxVertexCount = GS.maxVertexCount;
    if (maxVertexCount > DXIL::kMaxGSOutputVertexCount) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmGSOutputVertexCountRange,
                               {std::to_string(DXIL::kMaxGSOutputVertexCount),
                                std::to_string(maxVertexCount)});
    }

    unsigned instanceCount = GS.instanceCount;
    if (instanceCount > DXIL::kMaxGSInstanceCount || instanceCount < 1) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmGSInstanceCountRange,
                               {std::to_string(DXIL::kMaxGSInstanceCount),
                                std::to_string(instanceCount)});
    }

    DXIL::PrimitiveTopology topo = DXIL::PrimitiveTopology::Undefined;
    bool bTopoMismatch = false;
    for (size_t i = 0; i < _countof(GS.streamPrimitiveTopologies); ++i) {
      if (GS.streamPrimitiveTopologies[i] !=
          DXIL::PrimitiveTopology::Undefined) {
        if (topo == DXIL::PrimitiveTopology::Undefined)
          topo = GS.streamPrimitiveTopologies[i];
        else if (topo != GS.streamPrimitiveTopologies[i]) {
          bTopoMismatch = true;
          break;
        }
      }
    }
    if (bTopoMismatch)
      topo = DXIL::PrimitiveTopology::Undefined;
    switch (topo) {
    case DXIL::PrimitiveTopology::PointList:
    case DXIL::PrimitiveTopology::LineStrip:
    case DXIL::PrimitiveTopology::TriangleStrip:
      break;
    default: {
      ValCtx.EmitFnError(F, ValidationRule::SmGSValidOutputPrimitiveTopology);
    } break;
    }

    DXIL::InputPrimitive inputPrimitive = GS.inputPrimitive;
    unsigned VertexCount = GetNumVertices(inputPrimitive);
    if (VertexCount == 0 && inputPrimitive != DXIL::InputPrimitive::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmGSValidInputPrimitive);
    }
  }
}

static void ValidateShaderState(ValidationContext &ValCtx) {
  DxilModule &DM = ValCtx.DxilMod;
  if (ValCtx.isLibProfile) {
    for (Function &F : DM.GetModule()->functions()) {
      if (DM.HasDxilEntryProps(&F)) {
        DxilEntryProps &entryProps = DM.GetDxilEntryProps(&F);
        EntryStatus &Status = ValCtx.GetEntryStatus(&F);
        ValidateEntryProps(ValCtx, entryProps, Status, &F);
        ValidatePassThruHS(ValCtx, entryProps, &F);
      }
    }
  } else {
    Function *Entry = DM.GetEntryFunction();
    if (!DM.HasDxilEntryProps(Entry)) {
      // must have props.
      ValCtx.EmitFnError(Entry, ValidationRule::MetaNoEntryPropsForEntry);
      return;
    }
    EntryStatus &Status = ValCtx.GetEntryStatus(Entry);
    DxilEntryProps &entryProps = DM.GetDxilEntryProps(Entry);
    ValidateEntryProps(ValCtx, entryProps, Status, Entry);
    ValidatePassThruHS(ValCtx, entryProps, Entry);
  }
}

static CallGraphNode *
CalculateCallDepth(CallGraphNode *node,
                   std::unordered_map<CallGraphNode *, unsigned> &depthMap,
                   std::unordered_set<CallGraphNode *> &callStack,
                   std::unordered_set<Function *> &funcSet) {
  unsigned depth = callStack.size();
  funcSet.insert(node->getFunction());
  for (auto it = node->begin(), ei = node->end(); it != ei; it++) {
    CallGraphNode *toNode = it->second;
    if (callStack.insert(toNode).second == false) {
      // Recursive.
      return toNode;
    }
    if (depthMap[toNode] < depth)
      depthMap[toNode] = depth;
    if (CallGraphNode *N =
            CalculateCallDepth(toNode, depthMap, callStack, funcSet)) {
      // Recursive
      return N;
    }
    callStack.erase(toNode);
  }

  return nullptr;
}

static void ValidateCallGraph(ValidationContext &ValCtx) {
  // Build CallGraph.
  CallGraph &CG = ValCtx.GetCallGraph();

  std::unordered_map<CallGraphNode *, unsigned> depthMap;
  std::unordered_set<CallGraphNode *> callStack;
  CallGraphNode *entryNode = CG[ValCtx.DxilMod.GetEntryFunction()];
  depthMap[entryNode] = 0;
  if (CallGraphNode *N = CalculateCallDepth(entryNode, depthMap, callStack,
                                            ValCtx.entryFuncCallSet))
    ValCtx.EmitFnError(N->getFunction(), ValidationRule::FlowNoRecursion);
  if (ValCtx.DxilMod.GetShaderModel()->IsHS()) {
    CallGraphNode *patchConstantNode =
        CG[ValCtx.DxilMod.GetPatchConstantFunction()];
    depthMap[patchConstantNode] = 0;
    callStack.clear();
    if (CallGraphNode *N =
            CalculateCallDepth(patchConstantNode, depthMap, callStack,
                               ValCtx.patchConstFuncCallSet))
      ValCtx.EmitFnError(N->getFunction(), ValidationRule::FlowNoRecursion);
  }
}

static void ValidateFlowControl(ValidationContext &ValCtx) {
  bool reducible =
      IsReducible(*ValCtx.DxilMod.GetModule(), IrreducibilityAction::Ignore);
  if (!reducible) {
    ValCtx.EmitError(ValidationRule::FlowReducible);
    return;
  }

  ValidateCallGraph(ValCtx);

  for (llvm::Function &F : ValCtx.DxilMod.GetModule()->functions()) {
    if (F.isDeclaration())
      continue;

    DominatorTreeAnalysis DTA;
    DominatorTree DT = DTA.run(F);
    LoopInfo LI;
    LI.Analyze(DT);
    for (auto loopIt = LI.begin(); loopIt != LI.end(); loopIt++) {
      Loop *loop = *loopIt;
      SmallVector<BasicBlock *, 4> exitBlocks;
      loop->getExitBlocks(exitBlocks);
      if (exitBlocks.empty())
        ValCtx.EmitFnError(&F, ValidationRule::FlowDeadLoop);
    }

    // validate that there is no use of a value that has been output-completed
    // for this function.

    hlsl::OP *hlslOP = ValCtx.DxilMod.GetOP();

    for (auto &it : hlslOP->GetOpFuncList(DXIL::OpCode::OutputComplete)) {
      Function *pF = it.second;
      if (!pF)
        continue;

      // first, collect all the output complete calls that are not dominated
      // by another OutputComplete call for the same handle value
      llvm::SmallMapVector<Value *, llvm::SmallPtrSet<CallInst *, 4>, 4>
          handleToCI;
      for (User *U : pF->users()) {
        // all OutputComplete calls are instructions, and call instructions,
        // so there shouldn't need to be a null check.
        CallInst *CI = cast<CallInst>(U);

        // verify that the function that contains this instruction is the same
        // function that the DominatorTree was built on.
        if (&F != CI->getParent()->getParent())
          continue;

        DxilInst_OutputComplete OutputComplete(CI);
        Value *completedRecord = OutputComplete.get_output();

        auto vIt = handleToCI.find(completedRecord);
        if (vIt == handleToCI.end()) {
          llvm::SmallPtrSet<CallInst *, 4> s;
          s.insert(CI);
          handleToCI.insert(std::make_pair(completedRecord, s));
        } else {
          // if the handle is already in the map, make sure the map's set of
          // output complete calls that dominate the handle and do not dominate
          // each other gets updated if necessary
          bool CI_is_dominated = false;
          for (auto ocIt = vIt->second.begin(); ocIt != vIt->second.end();) {
            // if our new OC CI dominates an OC instruction in the set,
            // then replace the instruction in the set with the new OC CI.

            if (DT.dominates(CI, *ocIt)) {
              auto cur_it = ocIt++;
              vIt->second.erase(*cur_it);
              continue;
            }
            // Remember if our new CI gets dominated by any CI in the set.
            if (DT.dominates(*ocIt, CI)) {
              CI_is_dominated = true;
              break;
            }
            ocIt++;
          }
          // if no CI in the set dominates our new CI,
          // the new CI should be added to the set
          if (!CI_is_dominated)
            vIt->second.insert(CI);
        }
      }

      for (auto handle_iter = handleToCI.begin(), e = handleToCI.end();
           handle_iter != e; handle_iter++) {
        for (auto user_itr = handle_iter->first->user_begin();
             user_itr != handle_iter->first->user_end(); user_itr++) {
          User *pU = *user_itr;
          Instruction *useInstr = cast<Instruction>(pU);
          if (useInstr) {
            if (CallInst *CI = dyn_cast<CallInst>(useInstr)) {
              // if the user is an output complete call that is in the set of
              // OutputComplete calls not dominated by another OutputComplete
              // call for the same handle value, no diagnostics need to be
              // emitted.
              if (handle_iter->second.count(CI) == 1)
                continue;
            }

            // make sure any output complete call in the set
            // that dominates this use gets its diagnostic emitted.
            for (auto ocIt = handle_iter->second.begin();
                 ocIt != handle_iter->second.end(); ocIt++) {
              Instruction *ocInstr = cast<Instruction>(*ocIt);
              if (DT.dominates(ocInstr, useInstr)) {
                ValCtx.EmitInstrError(
                    useInstr,
                    ValidationRule::InstrNodeRecordHandleUseAfterComplete);
                ValCtx.EmitInstrNote(
                    *ocIt, "record handle invalidated by OutputComplete");
                break;
              }
            }
          }
        }
      }
    }
  }
  // fxc has ERR_CONTINUE_INSIDE_SWITCH to disallow continue in switch.
  // Not do it for now.
}

static void ValidateUninitializedOutput(ValidationContext &ValCtx,
                                        Function *F) {
  DxilModule &DM = ValCtx.DxilMod;
  DxilEntryProps &entryProps = DM.GetDxilEntryProps(F);
  EntryStatus &Status = ValCtx.GetEntryStatus(F);
  const DxilFunctionProps &props = entryProps.props;
  // For HS only need to check Tessfactor which is in patch constant sig.
  if (props.IsHS()) {
    std::vector<unsigned> &patchConstOrPrimCols = Status.patchConstOrPrimCols;
    const DxilSignature &patchConstSig =
        entryProps.sig.PatchConstOrPrimSignature;
    for (auto &E : patchConstSig.GetElements()) {
      unsigned mask = patchConstOrPrimCols[E->GetID()];
      unsigned requireMask = (1 << E->GetCols()) - 1;
      // TODO: check other case uninitialized output is allowed.
      if (mask != requireMask && !E->GetSemantic()->IsArbitrary()) {
        ValCtx.EmitFnFormatError(F, ValidationRule::SmUndefinedOutput,
                                 {E->GetName()});
      }
    }
    return;
  }
  const DxilSignature &outSig = entryProps.sig.OutputSignature;
  std::vector<unsigned> &outputCols = Status.outputCols;
  for (auto &E : outSig.GetElements()) {
    unsigned mask = outputCols[E->GetID()];
    unsigned requireMask = (1 << E->GetCols()) - 1;
    // TODO: check other case uninitialized output is allowed.
    if (mask != requireMask && !E->GetSemantic()->IsArbitrary() &&
        E->GetSemantic()->GetKind() != Semantic::Kind::Target) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmUndefinedOutput,
                               {E->GetName()});
    }
  }

  if (!props.IsGS()) {
    unsigned posMask = Status.OutputPositionMask[0];
    if (posMask != 0xf && Status.hasOutputPosition[0]) {
      ValCtx.EmitFnError(F, ValidationRule::SmCompletePosition);
    }
  } else {
    const auto &GS = props.ShaderProps.GS;
    unsigned streamMask = 0;
    for (size_t i = 0; i < _countof(GS.streamPrimitiveTopologies); ++i) {
      if (GS.streamPrimitiveTopologies[i] !=
          DXIL::PrimitiveTopology::Undefined) {
        streamMask |= 1 << i;
      }
    }

    for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
      if (streamMask & (1 << i)) {
        unsigned posMask = Status.OutputPositionMask[i];
        if (posMask != 0xf && Status.hasOutputPosition[i]) {
          ValCtx.EmitFnError(F, ValidationRule::SmCompletePosition);
        }
      }
    }
  }
}

static void ValidateUninitializedOutput(ValidationContext &ValCtx) {
  DxilModule &DM = ValCtx.DxilMod;
  if (ValCtx.isLibProfile) {
    for (Function &F : DM.GetModule()->functions()) {
      if (DM.HasDxilEntryProps(&F)) {
        ValidateUninitializedOutput(ValCtx, &F);
      }
    }
  } else {
    Function *Entry = DM.GetEntryFunction();
    if (!DM.HasDxilEntryProps(Entry)) {
      // must have props.
      ValCtx.EmitFnError(Entry, ValidationRule::MetaNoEntryPropsForEntry);
      return;
    }
    ValidateUninitializedOutput(ValCtx, Entry);
  }
}

uint32_t ValidateDxilModule(llvm::Module *pModule, llvm::Module *pDebugModule) {
  DxilModule *pDxilModule = DxilModule::TryGetDxilModule(pModule);
  if (!pDxilModule) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }
  if (pDxilModule->HasMetadataErrors()) {
    dxilutil::EmitErrorOnContext(pModule->getContext(),
                                 "Metadata error encountered in non-critical "
                                 "metadata (such as Type Annotations).");
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  ValidationContext ValCtx(*pModule, pDebugModule, *pDxilModule);

  ValidateBitcode(ValCtx);

  ValidateMetadata(ValCtx);

  ValidateShaderState(ValCtx);

  ValidateGlobalVariables(ValCtx);

  ValidateResources(ValCtx);

  // Validate control flow and collect function call info.
  // If has recursive call, call info collection will not finish.
  ValidateFlowControl(ValCtx);

  // Validate functions.
  for (Function &F : pModule->functions()) {
    ValidateFunction(F, ValCtx);
  }

  ValidateShaderFlags(ValCtx);

  ValidateEntryCompatibility(ValCtx);

  ValidateEntrySignatures(ValCtx);

  ValidateUninitializedOutput(ValCtx);
  // Ensure error messages are flushed out on error.
  if (ValCtx.Failed) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }
  return S_OK;
}

} // namespace hlsl
