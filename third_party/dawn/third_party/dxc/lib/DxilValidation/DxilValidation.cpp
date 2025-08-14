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
PrintDiagnosticContext::PrintDiagnosticContext(DiagnosticPrinter &Printer)
    : m_Printer(Printer), m_errorsFound(false), m_warningsFound(false) {}

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
                                        DxilSignatureElement &SE, Value *RowVal,
                                        Value *ColVal, EntryStatus &Status,
                                        ValidationContext &ValCtx) {
  if (ConstantInt *ConstRow = dyn_cast<ConstantInt>(RowVal)) {
    unsigned Row = ConstRow->getLimitedValue();
    if (Row >= SE.GetRows()) {
      std::string Range = std::string("0~") + std::to_string(SE.GetRows());
      ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOperandRange,
                                  {"Row", Range, std::to_string(Row)});
    }
  }

  if (!isa<ConstantInt>(ColVal)) {
    // Col must be const
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"Col", "LoadInput/StoreOutput"});
    return 0;
  }

  unsigned Col = cast<ConstantInt>(ColVal)->getLimitedValue();

  if (Col > SE.GetCols()) {
    std::string Range = std::string("0~") + std::to_string(SE.GetCols());
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOperandRange,
                                {"Col", Range, std::to_string(Col)});
  } else {
    if (SE.IsOutput())
      Status.outputCols[SE.GetID()] |= 1 << Col;
    if (SE.IsPatchConstOrPrim())
      Status.patchConstOrPrimCols[SE.GetID()] |= 1 << Col;
  }

  return Col;
}

static DxilSignatureElement *
ValidateSignatureAccess(Instruction *I, DxilSignature &Sig, Value *SigId,
                        Value *RowVal, Value *ColVal, EntryStatus &Status,
                        ValidationContext &ValCtx) {
  if (!isa<ConstantInt>(SigId)) {
    // inputID must be const
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"SignatureID", "LoadInput/StoreOutput"});
    return nullptr;
  }

  unsigned SEIdx = cast<ConstantInt>(SigId)->getLimitedValue();
  if (Sig.GetElements().size() <= SEIdx) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrOpConstRange);
    return nullptr;
  }

  DxilSignatureElement &SE = Sig.GetElement(SEIdx);
  bool IsOutput = Sig.IsOutput();

  unsigned Col = ValidateSignatureRowCol(I, SE, RowVal, ColVal, Status, ValCtx);

  if (IsOutput && SE.GetSemantic()->GetKind() == DXIL::SemanticKind::Position) {
    unsigned Mask = Status.OutputPositionMask[SE.GetOutputStream()];
    Mask |= 1 << Col;
    if (SE.GetOutputStream() < DXIL::kNumOutputStreams)
      Status.OutputPositionMask[SE.GetOutputStream()] = Mask;
  }
  return &SE;
}

static DxilResourceProperties GetResourceFromHandle(Value *Handle,
                                                    ValidationContext &ValCtx) {
  CallInst *HandleCall = dyn_cast<CallInst>(Handle);
  if (!HandleCall) {
    if (Instruction *I = dyn_cast<Instruction>(Handle))
      ValCtx.EmitInstrError(I, ValidationRule::InstrHandleNotFromCreateHandle);
    else
      ValCtx.EmitError(ValidationRule::InstrHandleNotFromCreateHandle);
    DxilResourceProperties RP;
    return RP;
  }

  DxilResourceProperties RP = ValCtx.GetResourceFromVal(Handle);
  if (RP.getResourceClass() == DXIL::ResourceClass::Invalid)
    ValCtx.EmitInstrError(cast<CallInst>(Handle),
                          ValidationRule::InstrHandleNotFromCreateHandle);
  if (RP.Basic.IsReorderCoherent &&
      !ValCtx.DxilMod.GetShaderModel()->IsSM69Plus())
    ValCtx.EmitInstrError(HandleCall,
                          ValidationRule::InstrReorderCoherentRequiresSM69);

  return RP;
}

static DXIL::SamplerKind GetSamplerKind(Value *SamplerHandle,
                                        ValidationContext &ValCtx) {
  DxilResourceProperties RP = GetResourceFromHandle(SamplerHandle, ValCtx);

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
GetResourceKindAndCompTy(Value *Handle, DXIL::ComponentType &CompTy,
                         DXIL::ResourceClass &ResClass,
                         ValidationContext &ValCtx) {
  CompTy = DXIL::ComponentType::Invalid;
  ResClass = DXIL::ResourceClass::Invalid;
  // TODO: validate ROV is used only in PS.

  DxilResourceProperties RP = GetResourceFromHandle(Handle, ValCtx);
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

DxilFieldAnnotation *GetFieldAnnotation(Type *Ty, DxilTypeSystem &TypeSys,
                                        std::deque<unsigned> &Offsets) {
  unsigned CurIdx = 1;
  unsigned LastIdx = Offsets.size() - 1;
  DxilStructAnnotation *StructAnnot = nullptr;

  for (; CurIdx < Offsets.size(); ++CurIdx) {
    if (const StructType *EltST = dyn_cast<StructType>(Ty)) {
      if (DxilStructAnnotation *EltAnnot = TypeSys.GetStructAnnotation(EltST)) {
        StructAnnot = EltAnnot;
        Ty = EltST->getElementType(Offsets[CurIdx]);
        if (CurIdx == LastIdx) {
          return &StructAnnot->GetFieldAnnotation(Offsets[CurIdx]);
        }
      } else {
        return nullptr;
      }
    } else if (const ArrayType *AT = dyn_cast<ArrayType>(Ty)) {
      Ty = AT->getElementType();
      StructAnnot = nullptr;
    } else {
      if (StructAnnot)
        return &StructAnnot->GetFieldAnnotation(Offsets[CurIdx]);
    }
  }
  return nullptr;
}

DxilResourceProperties ValidationContext::GetResourceFromVal(Value *ResVal) {
  auto It = ResPropMap.find(ResVal);
  if (It != ResPropMap.end()) {
    return It->second;
  } else {
    DxilResourceProperties RP;
    return RP;
  }
}

struct ResRetUsage {
  bool X;
  bool Y;
  bool Z;
  bool W;
  bool Status;
  ResRetUsage() : X(false), Y(false), Z(false), W(false), Status(false) {}
};

static void CollectGetDimResRetUsage(ResRetUsage &Usage, Instruction *ResRet,
                                     ValidationContext &ValCtx) {
  for (User *U : ResRet->users()) {
    if (ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(U)) {
      for (unsigned Idx : EVI->getIndices()) {
        switch (Idx) {
        case 0:
          Usage.X = true;
          break;
        case 1:
          Usage.Y = true;
          break;
        case 2:
          Usage.Z = true;
          break;
        case 3:
          Usage.W = true;
          break;
        case DXIL::kResRetStatusIndex:
          Usage.Status = true;
          break;
        default:
          // Emit index out of bound.
          ValCtx.EmitInstrError(EVI,
                                ValidationRule::InstrDxilStructUserOutOfBound);
          break;
        }
      }
    } else if (PHINode *PHI = dyn_cast<PHINode>(U)) {
      CollectGetDimResRetUsage(Usage, PHI, ValCtx);
    } else {
      Instruction *User = cast<Instruction>(U);
      ValCtx.EmitInstrError(User, ValidationRule::InstrDxilStructUser);
    }
  }
}

static void ValidateResourceCoord(CallInst *CI, DXIL::ResourceKind ResKind,
                                  ArrayRef<Value *> Coords,
                                  ValidationContext &ValCtx) {
  const unsigned KMaxNumCoords = 4;
  unsigned NumCoords = DxilResource::GetNumCoords(ResKind);
  for (unsigned I = 0; I < KMaxNumCoords; I++) {
    if (I < NumCoords) {
      if (isa<UndefValue>(Coords[I])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceCoordinateMiss);
      }
    } else {
      if (!isa<UndefValue>(Coords[I])) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrResourceCoordinateTooMany);
      }
    }
  }
}

static void ValidateCalcLODResourceDimensionCoord(CallInst *CI,
                                                  DXIL::ResourceKind ResKind,
                                                  ArrayRef<Value *> Coords,
                                                  ValidationContext &ValCtx) {
  const unsigned kMaxNumDimCoords = 3;
  unsigned NumCoords = DxilResource::GetNumDimensionsForCalcLOD(ResKind);
  for (unsigned I = 0; I < kMaxNumDimCoords; I++) {
    if (I < NumCoords) {
      if (isa<UndefValue>(Coords[I])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceCoordinateMiss);
      }
    } else {
      if (!isa<UndefValue>(Coords[I])) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrResourceCoordinateTooMany);
      }
    }
  }
}

static void ValidateResourceOffset(CallInst *CI, DXIL::ResourceKind ResKind,
                                   ArrayRef<Value *> Offsets,
                                   ValidationContext &ValCtx) {
  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();

  unsigned NumOffsets = DxilResource::GetNumOffsets(ResKind);
  bool HasOffset = !isa<UndefValue>(Offsets[0]);

  auto ValidateOffset = [&](Value *Offset) {
    // 6.7 Advanced Textures allow programmable offsets
    if (pSM->IsSM67Plus())
      return;
    if (ConstantInt *cOffset = dyn_cast<ConstantInt>(Offset)) {
      int Offset = cOffset->getValue().getSExtValue();
      if (Offset > 7 || Offset < -8) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrTextureOffset);
      }
    } else {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrTextureOffset);
    }
  };

  if (HasOffset) {
    ValidateOffset(Offsets[0]);
  }

  for (unsigned I = 1; I < Offsets.size(); I++) {
    if (I < NumOffsets) {
      if (HasOffset) {
        if (isa<UndefValue>(Offsets[I]))
          ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetMiss);
        else
          ValidateOffset(Offsets[I]);
      }
    } else {
      if (!isa<UndefValue>(Offsets[I])) {
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

static void ValidateSampleInst(CallInst *CI, Value *SrvHandle,
                               Value *SamplerHandle, ArrayRef<Value *> Coords,
                               ArrayRef<Value *> Offsets, bool IsSampleC,
                               ValidationContext &ValCtx) {
  if (!IsSampleC) {
    if (GetSamplerKind(SamplerHandle, ValCtx) != DXIL::SamplerKind::Default) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSample);
    }
  } else {
    if (GetSamplerKind(SamplerHandle, ValCtx) !=
        DXIL::SamplerKind::Comparison) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSampleC);
    }
  }

  DXIL::ComponentType CompTy;
  DXIL::ResourceClass ResClass;
  DXIL::ResourceKind ResKind =
      GetResourceKindAndCompTy(SrvHandle, CompTy, ResClass, ValCtx);
  bool IsSampleCompTy = CompTy == DXIL::ComponentType::F32;
  IsSampleCompTy |= CompTy == DXIL::ComponentType::SNormF32;
  IsSampleCompTy |= CompTy == DXIL::ComponentType::UNormF32;
  IsSampleCompTy |= CompTy == DXIL::ComponentType::F16;
  IsSampleCompTy |= CompTy == DXIL::ComponentType::SNormF16;
  IsSampleCompTy |= CompTy == DXIL::ComponentType::UNormF16;
  const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
  if (pSM->IsSM67Plus() && !IsSampleC) {
    IsSampleCompTy |= CompTy == DXIL::ComponentType::I16;
    IsSampleCompTy |= CompTy == DXIL::ComponentType::U16;
    IsSampleCompTy |= CompTy == DXIL::ComponentType::I32;
    IsSampleCompTy |= CompTy == DXIL::ComponentType::U32;
  }
  if (!IsSampleCompTy) {
    ValCtx.EmitInstrError(CI, ValidationRule::InstrSampleCompType);
  }

  if (ResClass != DXIL::ResourceClass::SRV) {
    ValCtx.EmitInstrError(CI,
                          ValidationRule::InstrResourceClassForSamplerGather);
  }

  ValidationRule Rule = ValidationRule::InstrResourceKindForSample;
  if (IsSampleC) {
    Rule = ValidationRule::InstrResourceKindForSampleC;
  }

  switch (ResKind) {
  case DXIL::ResourceKind::Texture1D:
  case DXIL::ResourceKind::Texture1DArray:
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture2DArray:
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::TextureCubeArray:
    break;
  case DXIL::ResourceKind::Texture3D:
    if (IsSampleC) {
      ValCtx.EmitInstrError(CI, Rule);
    }
    break;
  default:
    ValCtx.EmitInstrError(CI, Rule);
    return;
  }

  // Coord match resource kind.
  ValidateResourceCoord(CI, ResKind, Coords, ValCtx);
  // Offset match resource kind.
  ValidateResourceOffset(CI, ResKind, Offsets, ValCtx);
}

static void ValidateGather(CallInst *CI, Value *SrvHandle, Value *SamplerHandle,
                           ArrayRef<Value *> Coords, ArrayRef<Value *> Offsets,
                           bool IsSampleC, ValidationContext &ValCtx) {
  if (!IsSampleC) {
    if (GetSamplerKind(SamplerHandle, ValCtx) != DXIL::SamplerKind::Default) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSample);
    }
  } else {
    if (GetSamplerKind(SamplerHandle, ValCtx) !=
        DXIL::SamplerKind::Comparison) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForSampleC);
    }
  }

  DXIL::ComponentType CompTy;
  DXIL::ResourceClass ResClass;
  DXIL::ResourceKind ResKind =
      GetResourceKindAndCompTy(SrvHandle, CompTy, ResClass, ValCtx);

  if (ResClass != DXIL::ResourceClass::SRV) {
    ValCtx.EmitInstrError(CI,
                          ValidationRule::InstrResourceClassForSamplerGather);
    return;
  }

  // Coord match resource kind.
  ValidateResourceCoord(CI, ResKind, Coords, ValCtx);
  // Offset match resource kind.
  switch (ResKind) {
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture2DArray: {
    bool HasOffset = !isa<UndefValue>(Offsets[0]);
    if (HasOffset) {
      if (isa<UndefValue>(Offsets[1])) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetMiss);
      }
    }
  } break;
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::TextureCubeArray: {
    if (!isa<UndefValue>(Offsets[0])) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetTooMany);
    }
    if (!isa<UndefValue>(Offsets[1])) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceOffsetTooMany);
    }
  } break;
  default:
    // Invalid resource type for gather.
    ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForGather);
    return;
  }
}

static unsigned StoreValueToMask(ArrayRef<Value *> Vals) {
  unsigned Mask = 0;
  for (unsigned I = 0; I < 4; I++) {
    if (!isa<UndefValue>(Vals[I])) {
      Mask |= 1 << I;
    }
  }
  return Mask;
}

static int GetCBufSize(Value *CbHandle, ValidationContext &ValCtx) {
  DxilResourceProperties RP = GetResourceFromHandle(CbHandle, ValCtx);

  if (RP.getResourceClass() != DXIL::ResourceClass::CBuffer) {
    ValCtx.EmitInstrError(cast<CallInst>(CbHandle),
                          ValidationRule::InstrCBufferClassForCBufferHandle);
    return -1;
  }

  return RP.CBufferSizeInBytes;
}

// Make sure none of the handle arguments are undef / zero-initializer,
// Also, do not accept any resource handles with invalid dxil resource
// properties
void ValidateHandleArgsForInstruction(CallInst *CI, DXIL::OpCode Opcode,
                                      ValidationContext &ValCtx) {

  for (Value *op : CI->operands()) {
    const Type *pHandleTy = ValCtx.HandleTy; // This is a resource handle
    const Type *pNodeHandleTy = ValCtx.DxilMod.GetOP()->GetNodeHandleType();
    const Type *pNodeRecordHandleTy =
        ValCtx.DxilMod.GetOP()->GetNodeRecordHandleType();

    const Type *ArgTy = op->getType();
    if (ArgTy == pNodeHandleTy || ArgTy == pNodeRecordHandleTy ||
        ArgTy == pHandleTy) {

      if (isa<UndefValue>(op) || isa<ConstantAggregateZero>(op)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
      } else if (ArgTy == pHandleTy) {
        // GetResourceFromHandle will emit an error on an invalid handle
        GetResourceFromHandle(op, ValCtx);
      }
    }
  }
}

void ValidateHandleArgs(CallInst *CI, DXIL::OpCode Opcode,
                        ValidationContext &ValCtx) {

  switch (Opcode) {
    // TODO: add case DXIL::OpCode::IndexNodeRecordHandle:

  case DXIL::OpCode::AnnotateHandle:
  case DXIL::OpCode::AnnotateNodeHandle:
  case DXIL::OpCode::AnnotateNodeRecordHandle:
  case DXIL::OpCode::CreateHandleForLib:
    // TODO: add custom validation for these intrinsics
    break;

  default:
    ValidateHandleArgsForInstruction(CI, Opcode, ValCtx);
    break;
  }
}

static unsigned GetNumVertices(DXIL::InputPrimitive InputPrimitive) {
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

  unsigned PrimitiveIdx = static_cast<unsigned>(InputPrimitive);
  return InputPrimitiveVertexTab[PrimitiveIdx];
}

static void ValidateSignatureDxilOp(CallInst *CI, DXIL::OpCode Opcode,
                                    ValidationContext &ValCtx) {
  Function *F = CI->getParent()->getParent();
  DxilModule &DM = ValCtx.DxilMod;
  bool IsPatchConstantFunc = false;
  if (!DM.HasDxilEntryProps(F)) {
    auto It = ValCtx.PatchConstantFuncMap.find(F);
    if (It == ValCtx.PatchConstantFuncMap.end()) {
      // Missing entry props.
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrSignatureOperationNotInEntry);
      return;
    }
    // Use hull entry instead of patch constant function.
    F = It->second.front();
    IsPatchConstantFunc = true;
  }
  if (!ValCtx.HasEntryStatus(F)) {
    return;
  }

  EntryStatus &Status = ValCtx.GetEntryStatus(F);
  DxilEntryProps &EntryProps = DM.GetDxilEntryProps(F);
  DxilFunctionProps &Props = EntryProps.props;
  DxilEntrySignature &S = EntryProps.sig;

  switch (Opcode) {
  case DXIL::OpCode::LoadInput: {
    Value *InputId = CI->getArgOperand(DXIL::OperandIndex::kLoadInputIDOpIdx);
    DxilSignature &InputSig = S.InputSignature;
    Value *Row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *Col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    ValidateSignatureAccess(CI, InputSig, InputId, Row, Col, Status, ValCtx);

    // Check VertexId in ps/vs. and none array input.
    Value *VertexId =
        CI->getArgOperand(DXIL::OperandIndex::kLoadInputVertexIDOpIdx);
    bool UsedVertexId = VertexId && !isa<UndefValue>(VertexId);
    if (Props.IsVS() || Props.IsPS()) {
      if (UsedVertexId) {
        // Use VertexId in VS/PS input.
        ValCtx.EmitInstrError(CI, ValidationRule::SmOperand);
        return;
      }
    } else {
      if (ConstantInt *cVertexId = dyn_cast<ConstantInt>(VertexId)) {
        int ImmVertexId = cVertexId->getValue().getLimitedValue();
        if (cVertexId->getValue().isNegative()) {
          ImmVertexId = cVertexId->getValue().getSExtValue();
        }
        const int Low = 0;
        int High = 0;
        if (Props.IsGS()) {
          DXIL::InputPrimitive InputPrimitive =
              Props.ShaderProps.GS.inputPrimitive;
          High = GetNumVertices(InputPrimitive);
        } else if (Props.IsDS()) {
          High = Props.ShaderProps.DS.inputControlPoints;
        } else if (Props.IsHS()) {
          High = Props.ShaderProps.HS.inputControlPoints;
        } else {
          ValCtx.EmitInstrFormatError(CI,
                                      ValidationRule::SmOpcodeInInvalidFunction,
                                      {"LoadInput", "VS/HS/DS/GS/PS"});
        }
        if (ImmVertexId < Low || ImmVertexId >= High) {
          std::string Range = std::to_string(Low) + "~" + std::to_string(High);
          ValCtx.EmitInstrFormatError(
              CI, ValidationRule::InstrOperandRange,
              {"VertexID", Range, std::to_string(ImmVertexId)});
        }
      }
    }
  } break;
  case DXIL::OpCode::DomainLocation: {
    Value *ColValue =
        CI->getArgOperand(DXIL::OperandIndex::kDomainLocationColOpIdx);
    if (!isa<ConstantInt>(ColValue)) {
      // Col must be const
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                  {"Col", "DomainLocation"});
    } else {
      unsigned Col = cast<ConstantInt>(ColValue)->getLimitedValue();
      if (Col >= Status.domainLocSize) {
        ValCtx.EmitInstrError(CI, ValidationRule::SmDomainLocationIdxOOB);
      }
    }
  } break;
  case DXIL::OpCode::StoreOutput:
  case DXIL::OpCode::StoreVertexOutput:
  case DXIL::OpCode::StorePrimitiveOutput: {
    Value *OutputId =
        CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
    DxilSignature &OutputSig = Opcode == DXIL::OpCode::StorePrimitiveOutput
                                   ? S.PatchConstOrPrimSignature
                                   : S.OutputSignature;
    Value *Row = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
    Value *Col = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
    ValidateSignatureAccess(CI, OutputSig, OutputId, Row, Col, Status, ValCtx);
  } break;
  case DXIL::OpCode::OutputControlPointID: {
    // Only used in hull shader.
    Function *Func = CI->getParent()->getParent();
    // Make sure this is inside hs shader entry function.
    if (!(Props.IsHS() && F == Func)) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"OutputControlPointID", "hull function"});
    }
  } break;
  case DXIL::OpCode::LoadOutputControlPoint: {
    // Only used in patch constant function.
    Function *Func = CI->getParent()->getParent();
    if (ValCtx.entryFuncCallSet.count(Func) > 0) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"LoadOutputControlPoint", "PatchConstant function"});
    }
    Value *OutputId =
        CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
    DxilSignature &OutputSig = S.OutputSignature;
    Value *Row = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
    Value *Col = CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
    ValidateSignatureAccess(CI, OutputSig, OutputId, Row, Col, Status, ValCtx);
  } break;
  case DXIL::OpCode::StorePatchConstant: {
    // Only used in patch constant function.
    Function *Func = CI->getParent()->getParent();
    if (!IsPatchConstantFunc) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::SmOpcodeInInvalidFunction,
          {"StorePatchConstant", "PatchConstant function"});
    } else {
      auto &HullShaders = ValCtx.PatchConstantFuncMap[Func];
      for (Function *F : HullShaders) {
        EntryStatus &Status = ValCtx.GetEntryStatus(F);
        DxilEntryProps &EntryProps = DM.GetDxilEntryProps(F);
        DxilEntrySignature &S = EntryProps.sig;
        Value *OutputId =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputIDOpIdx);
        DxilSignature &OutputSig = S.PatchConstOrPrimSignature;
        Value *Row =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputRowOpIdx);
        Value *Col =
            CI->getArgOperand(DXIL::OperandIndex::kStoreOutputColOpIdx);
        ValidateSignatureAccess(CI, OutputSig, OutputId, Row, Col, Status,
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
    Value *InputId = CI->getArgOperand(DXIL::OperandIndex::kLoadInputIDOpIdx);
    DxilSignature &InputSig = S.InputSignature;
    Value *Row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *Col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    DxilSignatureElement *pSE = ValidateSignatureAccess(
        CI, InputSig, InputId, Row, Col, Status, ValCtx);
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
    DxilSignature &InputSig = S.InputSignature;
    Value *Row = CI->getArgOperand(DXIL::OperandIndex::kLoadInputRowOpIdx);
    Value *Col = CI->getArgOperand(DXIL::OperandIndex::kLoadInputColOpIdx);
    DxilSignatureElement *pSE = ValidateSignatureAccess(
        CI, InputSig, Attribute, Row, Col, Status, ValCtx);
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
    if (Props.IsGS()) {
      auto &GS = Props.ShaderProps.GS;
      unsigned StreamMask = 0;
      for (size_t I = 0; I < _countof(GS.streamPrimitiveTopologies); ++I) {
        if (GS.streamPrimitiveTopologies[I] !=
            DXIL::PrimitiveTopology::Undefined) {
          StreamMask |= 1 << I;
        }
      }
      Value *StreamId =
          CI->getArgOperand(DXIL::OperandIndex::kStreamEmitCutIDOpIdx);
      if (ConstantInt *cStreamId = dyn_cast<ConstantInt>(StreamId)) {
        int ImmStreamId = cStreamId->getValue().getLimitedValue();
        if (cStreamId->getValue().isNegative() || ImmStreamId >= 4) {
          ValCtx.EmitInstrFormatError(
              CI, ValidationRule::InstrOperandRange,
              {"StreamID", "0~4", std::to_string(ImmStreamId)});
        } else {
          unsigned ImmMask = 1 << ImmStreamId;
          if ((StreamMask & ImmMask) == 0) {
            std::string Range;
            for (unsigned I = 0; I < 4; I++) {
              if (StreamMask & (1 << I)) {
                Range += std::to_string(I) + " ";
              }
            }
            ValCtx.EmitInstrFormatError(
                CI, ValidationRule::InstrOperandRange,
                {"StreamID", Range, std::to_string(ImmStreamId)});
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
    if (!Props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"EmitIndices", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::SetMeshOutputCounts: {
    if (!Props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"SetMeshOutputCounts", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::GetMeshPayload: {
    if (!Props.IsMS()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"GetMeshPayload", "Mesh shader"});
    }
  } break;
  case DXIL::OpCode::DispatchMesh: {
    if (!Props.IsAS()) {
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

static void ValidateImmOperandForMathDxilOp(CallInst *CI, DXIL::OpCode Opcode,
                                            ValidationContext &ValCtx) {
  switch (Opcode) {
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

static bool CheckLinalgInterpretation(uint32_t Input, bool InRegister) {
  using CT = DXIL::ComponentType;
  switch (static_cast<CT>(Input)) {
  case CT::I16:
  case CT::U16:
  case CT::I32:
  case CT::U32:
  case CT::F16:
  case CT::F32:
  case CT::U8:
  case CT::I8:
  case CT::F8_E4M3:
  case CT::F8_E5M2:
    return true;
  case CT::PackedS8x32:
  case CT::PackedU8x32:
    return InRegister;
  default:
    return false;
  }
}

static bool CheckMatrixLayoutForMatVecMulOps(unsigned Layout) {
  return Layout <=
         static_cast<unsigned>(DXIL::LinalgMatrixLayout::OuterProductOptimal);
}

std::string GetMatrixLayoutStr(unsigned Layout) {
  switch (static_cast<DXIL::LinalgMatrixLayout>(Layout)) {
  case DXIL::LinalgMatrixLayout::RowMajor:
    return "RowMajor";
  case DXIL::LinalgMatrixLayout::ColumnMajor:
    return "ColumnMajor";
  case DXIL::LinalgMatrixLayout::MulOptimal:
    return "MulOptimal";
  case DXIL::LinalgMatrixLayout::OuterProductOptimal:
    return "OuterProductOptimal";
  default:
    DXASSERT_NOMSG(false);
    return "Invalid";
  }
}

static bool CheckTransposeForMatrixLayout(unsigned Layout, bool Transposed) {
  switch (static_cast<DXIL::LinalgMatrixLayout>(Layout)) {
  case DXIL::LinalgMatrixLayout::RowMajor:
  case DXIL::LinalgMatrixLayout::ColumnMajor:
    return !Transposed;

  default:
    return true;
  }
}

static bool CheckUnsignedFlag(Type *VecTy, bool IsUnsigned) {
  Type *ElemTy = VecTy->getScalarType();
  if (ElemTy->isFloatingPointTy())
    return !IsUnsigned;

  return true;
}

static Value *GetMatVecOpIsOutputUnsigned(CallInst *CI, DXIL::OpCode OpCode) {
  switch (OpCode) {
  case DXIL::OpCode::MatVecMul:
    return CI->getOperand(DXIL::OperandIndex::kMatVecMulIsOutputUnsignedIdx);
  case DXIL::OpCode::MatVecMulAdd:
    return CI->getOperand(DXIL::OperandIndex::kMatVecMulAddIsOutputUnsignedIdx);

  default:
    DXASSERT_NOMSG(false);
    return nullptr;
  }
}

static void ValidateImmOperandsForMatVecOps(CallInst *CI, DXIL::OpCode OpCode,
                                            ValidationContext &ValCtx) {

  llvm::Value *IsInputUnsigned =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulIsInputUnsignedIdx);
  ConstantInt *IsInputUnsignedConst =
      dyn_cast<llvm::ConstantInt>(IsInputUnsigned);
  if (!IsInputUnsignedConst) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrMatVecOpIsUnsignedFlagsAreConst,
        {"IsInputUnsigned"});
    return;
  }

  llvm::Value *IsOutputUnsigned = GetMatVecOpIsOutputUnsigned(CI, OpCode);
  ConstantInt *IsOutputUnsignedConst =
      dyn_cast<llvm::ConstantInt>(IsOutputUnsigned);
  if (!IsOutputUnsignedConst) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrMatVecOpIsUnsignedFlagsAreConst,
        {"IsOutputUnsigned"});
    return;
  }

  llvm::Value *InputInterpretation =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulInputInterpretationIdx);
  ConstantInt *II = dyn_cast<ConstantInt>(InputInterpretation);
  if (!II) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInterpretationParamAreConst,
        {"InputInterpretation"});
    return;
  }
  uint64_t IIValue = II->getLimitedValue();
  if (!CheckLinalgInterpretation(IIValue, true)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInvalidRegisterInterpValue,
        {std::to_string(IIValue), "Input"});
    return;
  }

  llvm::Value *MatrixInterpretation =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulMatrixInterpretationIdx);
  ConstantInt *MI = dyn_cast<ConstantInt>(MatrixInterpretation);
  if (!MI) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInterpretationParamAreConst,
        {"MatrixInterpretation"});
    return;
  }
  uint64_t MIValue = MI->getLimitedValue();
  if (!CheckLinalgInterpretation(MIValue, false)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInvalidMemoryInterpValue,
        {std::to_string(MIValue), "Matrix"});
    return;
  }

  llvm::Value *MatrixM =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulMatrixMIdx);
  if (!llvm::isa<llvm::Constant>(MatrixM)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixShapeParamsAreConst,
        {"Matrix M dimension"});
    return;
  }

  llvm::Value *MatrixK =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulMatrixKIdx);
  if (!llvm::isa<llvm::Constant>(MatrixK)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixShapeParamsAreConst,
        {"Matrix K dimension"});
    return;
  }

  llvm::Value *MatrixLayout =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulMatrixLayoutIdx);

  ConstantInt *MatrixLayoutConst = dyn_cast<ConstantInt>(MatrixLayout);
  if (!MatrixLayoutConst) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixShapeParamsAreConst,
        {"Matrix Layout"});
    return;
  }
  uint64_t MLValue = MatrixLayoutConst->getLimitedValue();
  if (!CheckMatrixLayoutForMatVecMulOps(MLValue)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInvalidMatrixLayoutValueForMatVecOps,
        {std::to_string(MLValue),
         std::to_string(
             static_cast<unsigned>(DXIL::LinalgMatrixLayout::RowMajor)),
         std::to_string(static_cast<unsigned>(
             DXIL::LinalgMatrixLayout::OuterProductOptimal))});
    return;
  }

  llvm::Value *MatrixTranspose =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulMatrixTransposeIdx);
  ConstantInt *MatrixTransposeConst = dyn_cast<ConstantInt>(MatrixTranspose);
  if (!MatrixTransposeConst) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixShapeParamsAreConst,
        {"MatrixTranspose"});
    return;
  }

  if (!CheckTransposeForMatrixLayout(MLValue,
                                     MatrixTransposeConst->getLimitedValue())) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixLayoutNotTransposable,
        {GetMatrixLayoutStr(MLValue)});
    return;
  }

  llvm::Value *InputVector =
      CI->getOperand(DXIL::OperandIndex::kMatVecMulInputVectorIdx);
  if (!CheckUnsignedFlag(InputVector->getType(),
                         IsInputUnsignedConst->getLimitedValue())) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgNotAnUnsignedType, {"Input"});
    return;
  }

  if (!CheckUnsignedFlag(CI->getType(),
                         IsOutputUnsignedConst->getLimitedValue())) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgNotAnUnsignedType, {"Output"});
    return;
  }

  switch (OpCode) {
  case DXIL::OpCode::MatVecMulAdd: {
    llvm::Value *BiasInterpretation =
        CI->getOperand(DXIL::OperandIndex::kMatVecMulAddBiasInterpretation);
    ConstantInt *BI = cast<ConstantInt>(BiasInterpretation);
    if (!BI) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::InstrLinalgInterpretationParamAreConst,
          {"BiasInterpretation"});
      return;
    }
    uint64_t BIValue = BI->getLimitedValue();
    if (!CheckLinalgInterpretation(BIValue, false)) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::InstrLinalgInvalidMemoryInterpValue,
          {std::to_string(BIValue), "Bias vector"});
      return;
    }
  } break;
  default:
    break;
  }
}

static void ValidateImmOperandsForOuterProdAcc(CallInst *CI,
                                               ValidationContext &ValCtx) {

  llvm::Value *MatrixInterpretation =
      CI->getOperand(DXIL::OperandIndex::kOuterProdAccMatrixInterpretation);
  ConstantInt *MI = cast<ConstantInt>(MatrixInterpretation);
  if (!MI) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInterpretationParamAreConst,
        {"MatrixInterpretation"});
    return;
  }
  uint64_t MIValue = MI->getLimitedValue();
  if (!CheckLinalgInterpretation(MIValue, false)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgInvalidMemoryInterpValue,
        {std::to_string(MIValue), "Matrix"});
    return;
  }

  llvm::Value *MatrixLayout =
      CI->getOperand(DXIL::OperandIndex::kOuterProdAccMatrixLayout);
  if (!llvm::isa<llvm::Constant>(MatrixLayout)) {
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrLinalgMatrixShapeParamsAreConst,
        {"MatrixLayout"});
    return;
  }
  ConstantInt *ML = cast<ConstantInt>(MatrixLayout);
  uint64_t MLValue = ML->getLimitedValue();
  if (MLValue !=
      static_cast<unsigned>(DXIL::LinalgMatrixLayout::OuterProductOptimal))
    ValCtx.EmitInstrFormatError(
        CI,
        ValidationRule::
            InstrLinalgInvalidMatrixLayoutValueForOuterProductAccumulate,
        {GetMatrixLayoutStr(MLValue),
         GetMatrixLayoutStr(static_cast<unsigned>(
             DXIL::LinalgMatrixLayout::OuterProductOptimal))});

  llvm::Value *MatrixStride =
      CI->getOperand(DXIL::OperandIndex::kOuterProdAccMatrixStride);
  if (!llvm::isa<llvm::Constant>(MatrixStride)) {
    ValCtx.EmitInstrError(
        CI, ValidationRule::InstrLinalgMatrixStrideZeroForOptimalLayouts);
    return;
  }
  ConstantInt *MS = cast<ConstantInt>(MatrixStride);
  uint64_t MSValue = MS->getLimitedValue();
  if (MSValue != 0) {
    ValCtx.EmitInstrError(
        CI, ValidationRule::InstrLinalgMatrixStrideZeroForOptimalLayouts);
    return;
  }
}

// Validate the type-defined mask compared to the store value mask which
// indicates which parts were defined returns true if caller should continue
// validation
static bool ValidateStorageMasks(Instruction *I, DXIL::OpCode Opcode,
                                 ConstantInt *Mask, unsigned StValMask,
                                 bool IsTyped, ValidationContext &ValCtx) {
  if (!Mask) {
    // Mask for buffer store should be immediate.
    ValCtx.EmitInstrFormatError(I, ValidationRule::InstrOpConst,
                                {"Mask", hlsl::OP::GetOpCodeName(Opcode)});
    return false;
  }

  unsigned UMask = Mask->getLimitedValue();
  if (IsTyped && UMask != 0xf) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrWriteMaskForTypedUAVStore);
  }

  // write mask must be contiguous (.x .xy .xyz or .xyzw)
  if (!((UMask == 0xf) || (UMask == 0x7) || (UMask == 0x3) || (UMask == 0x1))) {
    ValCtx.EmitInstrError(I, ValidationRule::InstrWriteMaskGapForUAV);
  }

  // If a bit is set in the UMask (expected values) that isn't set in StValMask
  // (user provided values) then the user failed to define some of the output
  // values.
  if (UMask & ~StValMask)
    ValCtx.EmitInstrError(I, ValidationRule::InstrUndefinedValueForUAVStore);
  else if (UMask != StValMask)
    ValCtx.EmitInstrFormatError(
        I, ValidationRule::InstrWriteMaskMatchValueForUAVStore,
        {std::to_string(UMask), std::to_string(StValMask)});

  return true;
}

static void ValidateASHandle(CallInst *CI, Value *Hdl,
                             ValidationContext &ValCtx) {
  DxilResourceProperties RP = ValCtx.GetResourceFromVal(Hdl);
  if (RP.getResourceClass() == DXIL::ResourceClass::Invalid ||
      RP.getResourceKind() != DXIL::ResourceKind::RTAccelerationStructure) {
    ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceKindForTraceRay);
  }
}

static void ValidateResourceDxilOp(CallInst *CI, DXIL::OpCode Opcode,
                                   ValidationContext &ValCtx) {
  switch (Opcode) {
  case DXIL::OpCode::GetDimensions: {
    DxilInst_GetDimensions GetDim(CI);
    Value *Handle = GetDim.get_handle();
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(Handle, CompTy, ResClass, ValCtx);

    // Check the result component use.
    ResRetUsage Usage;
    CollectGetDimResRetUsage(Usage, CI, ValCtx);

    // Mip level only for texture.
    switch (ResKind) {
    case DXIL::ResourceKind::Texture1D:
      if (Usage.Y) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"y", "Texture1D"});
      }
      if (Usage.Z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture1D"});
      }
      break;
    case DXIL::ResourceKind::Texture1DArray:
      if (Usage.Z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture1DArray"});
      }
      break;
    case DXIL::ResourceKind::Texture2D:
      if (Usage.Z) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrUndefResultForGetDimension,
            {"z", "Texture2D"});
      }
      break;
    case DXIL::ResourceKind::Texture2DArray:
      break;
    case DXIL::ResourceKind::Texture2DMS:
      if (Usage.Z) {
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
      if (Usage.Z) {
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
      Value *Mip = GetDim.get_mipLevel();
      if (!isa<UndefValue>(Mip)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrMipLevelForGetDimension);
      }
      if (ResKind != DXIL::ResourceKind::Invalid) {
        if (Usage.Y || Usage.Z || Usage.W) {
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

    if (Usage.Status) {
      ValCtx.EmitInstrFormatError(
          CI, ValidationRule::InstrUndefResultForGetDimension,
          {"invalid", "resource"});
    }
  } break;
  case DXIL::OpCode::CalculateLOD: {
    DxilInst_CalculateLOD LOD(CI);
    Value *SamplerHandle = LOD.get_sampler();
    DXIL::SamplerKind SamplerKind = GetSamplerKind(SamplerHandle, ValCtx);
    if (SamplerKind != DXIL::SamplerKind::Default) {
      // After SM68, Comparison is supported.
      if (!ValCtx.DxilMod.GetShaderModel()->IsSM68Plus() ||
          SamplerKind != DXIL::SamplerKind::Comparison)
        ValCtx.EmitInstrError(CI, ValidationRule::InstrSamplerModeForLOD);
    }
    Value *Handle = LOD.get_handle();
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(Handle, CompTy, ResClass, ValCtx);
    if (ResClass != DXIL::ResourceClass::SRV) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrResourceClassForSamplerGather);
      return;
    }
    // Coord match resource.
    ValidateCalcLODResourceDimensionCoord(
        CI, ResKind, {LOD.get_coord0(), LOD.get_coord1(), LOD.get_coord2()},
        ValCtx);

    switch (ResKind) {
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
    DxilInst_TextureGather Gather(CI);
    ValidateGather(CI, Gather.get_srv(), Gather.get_sampler(),
                   {Gather.get_coord0(), Gather.get_coord1(),
                    Gather.get_coord2(), Gather.get_coord3()},
                   {Gather.get_offset0(), Gather.get_offset1()},
                   /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::TextureGatherCmp: {
    DxilInst_TextureGatherCmp Gather(CI);
    ValidateGather(CI, Gather.get_srv(), Gather.get_sampler(),
                   {Gather.get_coord0(), Gather.get_coord1(),
                    Gather.get_coord2(), Gather.get_coord3()},
                   {Gather.get_offset0(), Gather.get_offset1()},
                   /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::Sample: {
    DxilInst_Sample Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmp: {
    DxilInst_SampleCmp Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpLevel: {
    // sampler must be comparison mode.
    DxilInst_SampleCmpLevel Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpLevelZero: {
    // sampler must be comparison mode.
    DxilInst_SampleCmpLevelZero Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleBias: {
    DxilInst_SampleBias Sample(CI);
    Value *Bias = Sample.get_bias();
    if (ConstantFP *cBias = dyn_cast<ConstantFP>(Bias)) {
      float FBias = cBias->getValueAPF().convertToFloat();
      if (FBias < DXIL::kMinMipLodBias || FBias > DXIL::kMaxMipLodBias) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrImmBiasForSampleB,
            {std::to_string(DXIL::kMinMipLodBias),
             std::to_string(DXIL::kMaxMipLodBias),
             std::to_string(cBias->getValueAPF().convertToFloat())});
      }
    }

    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpBias: {
    DxilInst_SampleCmpBias Sample(CI);
    Value *Bias = Sample.get_bias();
    if (ConstantFP *cBias = dyn_cast<ConstantFP>(Bias)) {
      float FBias = cBias->getValueAPF().convertToFloat();
      if (FBias < DXIL::kMinMipLodBias || FBias > DXIL::kMaxMipLodBias) {
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrImmBiasForSampleB,
            {std::to_string(DXIL::kMinMipLodBias),
             std::to_string(DXIL::kMaxMipLodBias),
             std::to_string(cBias->getValueAPF().convertToFloat())});
      }
    }

    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
    ValidateDerivativeOp(CI, ValCtx);
  } break;
  case DXIL::OpCode::SampleGrad: {
    DxilInst_SampleGrad Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::SampleCmpGrad: {
    DxilInst_SampleCmpGrad Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ true, ValCtx);
  } break;
  case DXIL::OpCode::SampleLevel: {
    DxilInst_SampleLevel Sample(CI);
    ValidateSampleInst(
        CI, Sample.get_srv(), Sample.get_sampler(),
        {Sample.get_coord0(), Sample.get_coord1(), Sample.get_coord2(),
         Sample.get_coord3()},
        {Sample.get_offset0(), Sample.get_offset1(), Sample.get_offset2()},
        /*IsSampleC*/ false, ValCtx);
  } break;
  case DXIL::OpCode::CheckAccessFullyMapped: {
    Value *Src = CI->getArgOperand(DXIL::OperandIndex::kUnarySrc0OpIdx);
    ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(Src);
    if (!EVI) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrCheckAccessFullyMapped);
    } else {
      Value *V = EVI->getOperand(0);
      StructType *StrTy = dyn_cast<StructType>(V->getType());
      unsigned ExtractIndex = EVI->getIndices()[0];
      // Ensure parameter is a single value that is extracted from the correct
      // ResRet struct location.
      bool IsLegal = EVI->getNumIndices() == 1 &&
                     (ExtractIndex == DXIL::kResRetStatusIndex ||
                      ExtractIndex == DXIL::kVecResRetStatusIndex) &&
                     ValCtx.DxilMod.GetOP()->IsResRetType(StrTy) &&
                     ExtractIndex == StrTy->getNumElements() - 1;
      if (!IsLegal) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCheckAccessFullyMapped);
      }
    }
  } break;
  case DXIL::OpCode::BufferStore: {
    DxilInst_BufferStore BufSt(CI);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(BufSt.get_uav(), CompTy, ResClass, ValCtx);

    if (ResClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);
    }

    ConstantInt *Mask = dyn_cast<ConstantInt>(BufSt.get_mask());
    unsigned StValMask =
        StoreValueToMask({BufSt.get_value0(), BufSt.get_value1(),
                          BufSt.get_value2(), BufSt.get_value3()});

    if (!ValidateStorageMasks(CI, Opcode, Mask, StValMask,
                              ResKind == DXIL::ResourceKind::TypedBuffer ||
                                  ResKind == DXIL::ResourceKind::TBuffer,
                              ValCtx))
      return;
    Value *Offset = BufSt.get_coord1();

    switch (ResKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(Offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::TypedBuffer:
    case DXIL::ResourceKind::TBuffer:
      if (!isa<UndefValue>(Offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(Offset)) {
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
    DxilInst_TextureStore TexSt(CI);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(TexSt.get_srv(), CompTy, ResClass, ValCtx);

    if (ResClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);
    }

    ConstantInt *Mask = dyn_cast<ConstantInt>(TexSt.get_mask());
    unsigned StValMask =
        StoreValueToMask({TexSt.get_value0(), TexSt.get_value1(),
                          TexSt.get_value2(), TexSt.get_value3()});

    if (!ValidateStorageMasks(CI, Opcode, Mask, StValMask, true /*IsTyped*/,
                              ValCtx))
      return;

    switch (ResKind) {
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
    DxilInst_BufferLoad BufLd(CI);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(BufLd.get_srv(), CompTy, ResClass, ValCtx);

    if (ResClass != DXIL::ResourceClass::SRV &&
        ResClass != DXIL::ResourceClass::UAV) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);
    }

    Value *Offset = BufLd.get_wot();

    switch (ResKind) {
    case DXIL::ResourceKind::RawBuffer:
    case DXIL::ResourceKind::TypedBuffer:
    case DXIL::ResourceKind::TBuffer:
      if (!isa<UndefValue>(Offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(Offset)) {
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
    DxilInst_TextureLoad TexLd(CI);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(TexLd.get_srv(), CompTy, ResClass, ValCtx);

    Value *MipLevel = TexLd.get_mipLevelOrSampleCount();

    if (ResClass == DXIL::ResourceClass::UAV) {
      bool NoOffset = isa<UndefValue>(TexLd.get_offset0());
      NoOffset &= isa<UndefValue>(TexLd.get_offset1());
      NoOffset &= isa<UndefValue>(TexLd.get_offset2());
      if (!NoOffset) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrOffsetOnUAVLoad);
      }
      if (!isa<UndefValue>(MipLevel)) {
        if (ResKind != DXIL::ResourceKind::Texture2DMS &&
            ResKind != DXIL::ResourceKind::Texture2DMSArray)
          ValCtx.EmitInstrError(CI, ValidationRule::InstrMipOnUAVLoad);
      }
    } else {
      if (ResClass != DXIL::ResourceClass::SRV) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);
      }
    }

    switch (ResKind) {
    case DXIL::ResourceKind::Texture1D:
    case DXIL::ResourceKind::Texture1DArray:
    case DXIL::ResourceKind::Texture2D:
    case DXIL::ResourceKind::Texture2DArray:
    case DXIL::ResourceKind::Texture3D:
      break;
    case DXIL::ResourceKind::Texture2DMS:
    case DXIL::ResourceKind::Texture2DMSArray: {
      if (isa<UndefValue>(MipLevel)) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrSampleIndexForLoad2DMS);
      }
    } break;
    default:
      ValCtx.EmitInstrError(CI,
                            ValidationRule::InstrResourceKindForTextureLoad);
      return;
    }

    ValidateResourceOffset(
        CI, ResKind,
        {TexLd.get_offset0(), TexLd.get_offset1(), TexLd.get_offset2()},
        ValCtx);
  } break;
  case DXIL::OpCode::CBufferLoad: {
    DxilInst_CBufferLoad CBLoad(CI);
    Value *RegIndex = CBLoad.get_byteOffset();
    if (ConstantInt *cIndex = dyn_cast<ConstantInt>(RegIndex)) {
      int Offset = cIndex->getLimitedValue();
      int Size = GetCBufSize(CBLoad.get_handle(), ValCtx);
      if (Size > 0 && Offset >= Size) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCBufferOutOfBound);
      }
    }
  } break;
  case DXIL::OpCode::CBufferLoadLegacy: {
    DxilInst_CBufferLoadLegacy CBLoad(CI);
    Value *RegIndex = CBLoad.get_regIndex();
    if (ConstantInt *cIndex = dyn_cast<ConstantInt>(RegIndex)) {
      int Offset = cIndex->getLimitedValue() * 16; // 16 bytes align
      int Size = GetCBufSize(CBLoad.get_handle(), ValCtx);
      if (Size > 0 && Offset >= Size) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrCBufferOutOfBound);
      }
    }
  } break;
  case DXIL::OpCode::RawBufferLoad:
    if (!ValCtx.DxilMod.GetShaderModel()->IsSM63Plus()) {
      Type *Ty = OP::GetOverloadType(DXIL::OpCode::RawBufferLoad,
                                     CI->getCalledFunction());
      if (ValCtx.DL.getTypeAllocSizeInBits(Ty) > 32)
        ValCtx.EmitInstrError(CI, ValidationRule::Sm64bitRawBufferLoadStore);
    }
    LLVM_FALLTHROUGH;
  case DXIL::OpCode::RawBufferVectorLoad: {
    Value *Handle =
        CI->getOperand(DXIL::OperandIndex::kRawBufferLoadHandleOpIdx);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(Handle, CompTy, ResClass, ValCtx);

    if (ResClass != DXIL::ResourceClass::SRV &&
        ResClass != DXIL::ResourceClass::UAV)

      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForLoad);

    unsigned AlignIdx = DXIL::OperandIndex::kRawBufferLoadAlignmentOpIdx;
    if (DXIL::OpCode::RawBufferVectorLoad == Opcode)
      AlignIdx = DXIL::OperandIndex::kRawBufferVectorLoadAlignmentOpIdx;
    if (!isa<ConstantInt>(CI->getOperand(AlignIdx)))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrConstAlignForRawBuf);

    Value *Offset =
        CI->getOperand(DXIL::OperandIndex::kRawBufferLoadElementOffsetOpIdx);
    switch (ResKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(Offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(Offset)) {
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
      if (ValCtx.DL.getTypeAllocSizeInBits(Ty) > 32)
        ValCtx.EmitInstrError(CI, ValidationRule::Sm64bitRawBufferLoadStore);
    }
    DxilInst_RawBufferStore bufSt(CI);
    ConstantInt *Mask = dyn_cast<ConstantInt>(bufSt.get_mask());
    unsigned StValMask =
        StoreValueToMask({bufSt.get_value0(), bufSt.get_value1(),
                          bufSt.get_value2(), bufSt.get_value3()});

    if (!ValidateStorageMasks(CI, Opcode, Mask, StValMask, false /*IsTyped*/,
                              ValCtx))
      return;
  }
    LLVM_FALLTHROUGH;
  case DXIL::OpCode::RawBufferVectorStore: {
    Value *Handle =
        CI->getOperand(DXIL::OperandIndex::kRawBufferStoreHandleOpIdx);
    DXIL::ComponentType CompTy;
    DXIL::ResourceClass ResClass;
    DXIL::ResourceKind ResKind =
        GetResourceKindAndCompTy(Handle, CompTy, ResClass, ValCtx);

    if (ResClass != DXIL::ResourceClass::UAV)
      ValCtx.EmitInstrError(CI, ValidationRule::InstrResourceClassForUAVStore);

    unsigned AlignIdx = DXIL::OperandIndex::kRawBufferStoreAlignmentOpIdx;
    if (DXIL::OpCode::RawBufferVectorStore == Opcode) {
      AlignIdx = DXIL::OperandIndex::kRawBufferVectorStoreAlignmentOpIdx;
      unsigned ValueIx = DXIL::OperandIndex::kRawBufferVectorStoreValOpIdx;
      if (isa<UndefValue>(CI->getOperand(ValueIx)))
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrUndefinedValueForUAVStore);
    }
    if (!isa<ConstantInt>(CI->getOperand(AlignIdx)))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrConstAlignForRawBuf);

    Value *Offset =
        CI->getOperand(DXIL::OperandIndex::kRawBufferStoreElementOffsetOpIdx);
    switch (ResKind) {
    case DXIL::ResourceKind::RawBuffer:
      if (!isa<UndefValue>(Offset)) {
        ValCtx.EmitInstrError(
            CI, ValidationRule::InstrCoordinateCountForRawTypedBuf);
      }
      break;
    case DXIL::ResourceKind::StructuredBuffer:
      if (isa<UndefValue>(Offset)) {
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
    DxilInst_TraceRay TraceRay(CI);
    Value *Hdl = TraceRay.get_AccelerationStructure();
    ValidateASHandle(CI, Hdl, ValCtx);
  } break;
  case DXIL::OpCode::HitObject_TraceRay: {
    DxilInst_HitObject_TraceRay HOTraceRay(CI);
    Value *Hdl = HOTraceRay.get_accelerationStructure();
    ValidateASHandle(CI, Hdl, ValCtx);
  } break;
  default:
    break;
  }
}

static void ValidateBarrierFlagArg(ValidationContext &ValCtx, CallInst *CI,
                                   Value *Arg, unsigned ValidMask,
                                   StringRef FlagName, StringRef OpName) {
  if (ConstantInt *CArg = dyn_cast<ConstantInt>(Arg)) {
    if ((CArg->getLimitedValue() & (uint32_t)(~ValidMask)) != 0) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrBarrierFlagInvalid,
                                  {FlagName, OpName});
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

static unsigned getSemanticFlagValidMask(const ShaderModel *pSM) {
  unsigned DxilMajor, DxilMinor;
  pSM->GetDxilVersion(DxilMajor, DxilMinor);
  // DXIL version >= 1.9
  if (hlsl::DXIL::CompareVersions(DxilMajor, DxilMinor, 1, 9) < 0)
    return static_cast<unsigned>(hlsl::DXIL::BarrierSemanticFlag::LegacyFlags);
  return static_cast<unsigned>(hlsl::DXIL::BarrierSemanticFlag::ValidMask);
}

StringRef GetOpCodeName(DXIL::OpCode OpCode) {
  switch (OpCode) {
  default:
    DXASSERT(false, "Unexpected op code");
    return "";
  case DXIL::OpCode::HitObject_ObjectRayOrigin:
    return "HitObject_ObjectRayOrigin";
  case DXIL::OpCode::HitObject_WorldRayDirection:
    return "HitObject_WorldRayDirection";
  case DXIL::OpCode::HitObject_WorldRayOrigin:
    return "HitObject_WorldRayOrigin";
  case DXIL::OpCode::HitObject_ObjectRayDirection:
    return "HitObject_ObjectRayDirection";
  case DXIL::OpCode::HitObject_WorldToObject3x4:
    return "HitObject_WorldToObject3x4";
  case DXIL::OpCode::HitObject_ObjectToWorld3x4:
    return "HitObject_ObjectToWorld3x4";
  }
}

static void ValidateConstantRangeUnsigned(Value *Val, StringRef Name,
                                          uint64_t LowerBound,
                                          uint64_t UpperBound, CallInst *CI,
                                          DXIL::OpCode OpCode,
                                          ValidationContext &ValCtx) {
  ConstantInt *C = dyn_cast<ConstantInt>(Val);
  if (!C) {
    ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                {Name, GetOpCodeName(OpCode)});
    return;
  }
  if (C->uge(UpperBound + 1U) || !C->uge(LowerBound)) {
    std::string Range =
        std::to_string(LowerBound) + "~" + std::to_string(UpperBound);
    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrOperandRange,
        {Name, Range, C->getValue().toString(10, false)});
  }
}

static void ValidateDxilOperationCallInProfile(CallInst *CI,
                                               DXIL::OpCode Opcode,
                                               const ShaderModel *pSM,
                                               ValidationContext &ValCtx) {
  DXIL::ShaderKind ShaderKind =
      pSM ? pSM->GetKind() : DXIL::ShaderKind::Invalid;
  llvm::Function *F = CI->getParent()->getParent();
  DXIL::NodeLaunchType NodeLaunchType = DXIL::NodeLaunchType::Invalid;
  if (DXIL::ShaderKind::Library == ShaderKind) {
    if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
      DxilEntryProps &EntryProps = ValCtx.DxilMod.GetDxilEntryProps(F);
      ShaderKind = ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
      if (ShaderKind == DXIL::ShaderKind::Node)
        NodeLaunchType = EntryProps.props.Node.LaunchType;

    } else if (ValCtx.DxilMod.IsPatchConstantShader(F))
      ShaderKind = DXIL::ShaderKind::Hull;
  }

  // These shader models are treted like compute
  bool IsCSLike = ShaderKind == DXIL::ShaderKind::Compute ||
                  ShaderKind == DXIL::ShaderKind::Mesh ||
                  ShaderKind == DXIL::ShaderKind::Amplification ||
                  ShaderKind == DXIL::ShaderKind::Node;
  // Is called from a library function
  bool IsLibFunc = ShaderKind == DXIL::ShaderKind::Library;

  ValidateHandleArgs(CI, Opcode, ValCtx);

  switch (Opcode) {
  // Imm input value validation.
  case DXIL::OpCode::Asin:
  case DXIL::OpCode::Acos:
  case DXIL::OpCode::Log:
  case DXIL::OpCode::DerivFineX:
  case DXIL::OpCode::DerivFineY:
  case DXIL::OpCode::DerivCoarseX:
  case DXIL::OpCode::DerivCoarseY:
    ValidateImmOperandForMathDxilOp(CI, Opcode, ValCtx);
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
  case DXIL::OpCode::RawBufferVectorLoad:
  case DXIL::OpCode::RawBufferVectorStore:
    ValidateResourceDxilOp(CI, Opcode, ValCtx);
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
    ValidateSignatureDxilOp(CI, Opcode, ValCtx);
    break;
  // Special.
  case DXIL::OpCode::AllocateRayQuery: {
    // validate flags are immediate and compatible
    llvm::Value *ConstRayFlag = CI->getOperand(1);
    if (!llvm::isa<llvm::Constant>(ConstRayFlag)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::DeclAllocateRayQueryFlagsAreConst);
    }
    break;
  }
  case DXIL::OpCode::AllocateRayQuery2: {
    // validate flags are immediate and compatible
    llvm::Value *ConstRayFlag = CI->getOperand(1);
    llvm::Value *RayQueryFlag = CI->getOperand(2);
    if (!llvm::isa<llvm::Constant>(ConstRayFlag) ||
        !llvm::isa<llvm::Constant>(RayQueryFlag)) {
      ValCtx.EmitInstrError(CI,
                            ValidationRule::DeclAllocateRayQuery2FlagsAreConst);
      break;
    }
    // When the ForceOMM2State ConstRayFlag is given as an argument to
    // a RayQuery object, AllowOpacityMicromaps is expected
    // as a RayQueryFlag argument
    llvm::ConstantInt *Arg1 = llvm::cast<llvm::ConstantInt>(ConstRayFlag);
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
    DxilInst_BufferUpdateCounter UpdateCounter(CI);
    Value *Handle = UpdateCounter.get_uav();
    DxilResourceProperties RP = ValCtx.GetResourceFromVal(Handle);

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

    Value *Inc = UpdateCounter.get_inc();
    if (ConstantInt *cInc = dyn_cast<ConstantInt>(Inc)) {
      bool IsInc = cInc->getLimitedValue() == 1;
      if (!ValCtx.isLibProfile) {
        auto It = ValCtx.HandleResIndexMap.find(Handle);
        if (It != ValCtx.HandleResIndexMap.end()) {
          unsigned ResIndex = It->second;
          if (ValCtx.UavCounterIncMap.count(ResIndex)) {
            if (IsInc != ValCtx.UavCounterIncMap[ResIndex]) {
              ValCtx.EmitInstrError(CI,
                                    ValidationRule::InstrOnlyOneAllocConsume);
            }
          } else {
            ValCtx.UavCounterIncMap[ResIndex] = IsInc;
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
    DxilInst_Barrier Barrier(CI);
    Value *Mode = Barrier.get_barrierMode();
    ConstantInt *CMode = dyn_cast<ConstantInt>(Mode);
    if (!CMode) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrOpConst,
                                  {"Mode", "Barrier"});
      return;
    }

    const unsigned Uglobal =
        static_cast<unsigned>(DXIL::BarrierMode::UAVFenceGlobal);
    const unsigned G = static_cast<unsigned>(DXIL::BarrierMode::TGSMFence);
    const unsigned Ut =
        static_cast<unsigned>(DXIL::BarrierMode::UAVFenceThreadGroup);
    unsigned BarrierMode = CMode->getLimitedValue();

    if (IsCSLike || IsLibFunc) {
      bool HasUGlobal = BarrierMode & Uglobal;
      bool HasGroup = BarrierMode & G;
      bool HasUGroup = BarrierMode & Ut;
      if (HasUGlobal && HasUGroup) {
        ValCtx.EmitInstrError(CI,
                              ValidationRule::InstrBarrierModeUselessUGroup);
      }
      if (!HasUGlobal && !HasGroup && !HasUGroup) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeNoMemory);
      }
    } else {
      if (Uglobal != BarrierMode) {
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
                           getSemanticFlagValidMask(pSM), "semantic",
                           "BarrierByMemoryType");
    if (!IsLibFunc && ShaderKind != DXIL::ShaderKind::Node &&
        OP::BarrierRequiresNode(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierRequiresNode);
    }
    if (!IsCSLike && !IsLibFunc && OP::BarrierRequiresGroup(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeForNonCS);
    }
  } break;
  case DXIL::OpCode::BarrierByNodeRecordHandle:
  case DXIL::OpCode::BarrierByMemoryHandle: {
    std::string OpName = Opcode == DXIL::OpCode::BarrierByNodeRecordHandle
                             ? "barrierByNodeRecordHandle"
                             : "barrierByMemoryHandle";
    DxilInst_BarrierByMemoryHandle DIMH(CI);
    ValidateBarrierFlagArg(ValCtx, CI, DIMH.get_SemanticFlags(),
                           getSemanticFlagValidMask(pSM), "semantic", OpName);
    if (!IsLibFunc && ShaderKind != DXIL::ShaderKind::Node &&
        OP::BarrierRequiresNode(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierRequiresNode);
    }
    if (!IsCSLike && !IsLibFunc && OP::BarrierRequiresGroup(CI)) {
      ValCtx.EmitInstrError(CI, ValidationRule::InstrBarrierModeForNonCS);
    }
  } break;
  case DXIL::OpCode::CreateHandleForLib:
    if (!ValCtx.isLibProfile) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::SmOpcodeInInvalidFunction,
                                  {"CreateHandleForLib", "Library"});
    }
    break;

  // Shader Execution Reordering
  case DXIL::OpCode::MaybeReorderThread: {
    Value *HitObject = CI->getArgOperand(1);
    Value *CoherenceHintBits = CI->getArgOperand(2);
    Value *NumCoherenceHintBits = CI->getArgOperand(3);

    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);

    if (isa<UndefValue>(NumCoherenceHintBits))
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrMayReorderThreadUndefCoherenceHintParam);

    ConstantInt *NumCoherenceHintBitsConst =
        dyn_cast<ConstantInt>(NumCoherenceHintBits);
    const bool HasCoherenceHint =
        NumCoherenceHintBitsConst &&
        NumCoherenceHintBitsConst->getLimitedValue() != 0;
    if (HasCoherenceHint && isa<UndefValue>(CoherenceHintBits))
      ValCtx.EmitInstrError(
          CI, ValidationRule::InstrMayReorderThreadUndefCoherenceHintParam);
  } break;
  case DXIL::OpCode::HitObject_MakeMiss: {
    DxilInst_HitObject_MakeMiss MakeMiss(CI);
    if (isa<UndefValue>(MakeMiss.get_RayFlags()) ||
        isa<UndefValue>(MakeMiss.get_MissShaderIndex()))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
  } break;

  case DXIL::OpCode::HitObject_LoadLocalRootTableConstant: {
    Value *HitObject = CI->getArgOperand(1);
    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    Value *Offset = CI->getArgOperand(2);
    if (isa<UndefValue>(Offset))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
    if (ConstantInt *COffset = dyn_cast<ConstantInt>(Offset)) {
      if (COffset->getLimitedValue() % 4 != 0)
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::InstrParamMultiple,
            {"offset", "4", COffset->getValue().toString(10, false)});
    }
    break;
  }
  case DXIL::OpCode::HitObject_SetShaderTableIndex: {
    Value *HitObject = CI->getArgOperand(1);
    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    Value *RecordIndex = CI->getArgOperand(2);
    if (isa<UndefValue>(RecordIndex))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
    break;
  }

  // Shader Execution Reordering - scalar getters
  case DXIL::OpCode::HitObject_GeometryIndex:
  case DXIL::OpCode::HitObject_HitKind:
  case DXIL::OpCode::HitObject_InstanceID:
  case DXIL::OpCode::HitObject_InstanceIndex:
  case DXIL::OpCode::HitObject_IsHit:
  case DXIL::OpCode::HitObject_IsMiss:
  case DXIL::OpCode::HitObject_IsNop:
  case DXIL::OpCode::HitObject_PrimitiveIndex:
  case DXIL::OpCode::HitObject_RayFlags:
  case DXIL::OpCode::HitObject_RayTCurrent:
  case DXIL::OpCode::HitObject_RayTMin:
  case DXIL::OpCode::HitObject_ShaderTableIndex: {
    Value *HitObject = CI->getArgOperand(1);
    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    break;
  }

  // Shader Execution Reordering - vector getters
  case DXIL::OpCode::HitObject_ObjectRayDirection:
  case DXIL::OpCode::HitObject_ObjectRayOrigin:
  case DXIL::OpCode::HitObject_WorldRayDirection:
  case DXIL::OpCode::HitObject_WorldRayOrigin: {
    Value *HitObject = CI->getArgOperand(1);
    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    Value *Col = CI->getArgOperand(2);
    ValidateConstantRangeUnsigned(Col, "component", 0, 2, CI, Opcode, ValCtx);
    break;
  }

  // Shader Execution Reordering - matrix getters
  case DXIL::OpCode::HitObject_WorldToObject3x4:
  case DXIL::OpCode::HitObject_ObjectToWorld3x4: {
    Value *HitObject = CI->getArgOperand(1);
    if (isa<UndefValue>(HitObject))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    Value *Row = CI->getArgOperand(2);
    ValidateConstantRangeUnsigned(Row, "row", 0, 2, CI, Opcode, ValCtx);
    Value *Col = CI->getArgOperand(3);
    ValidateConstantRangeUnsigned(Col, "column", 0, 3, CI, Opcode, ValCtx);
    break;
  }

  // Shader Execution Reordering - from ray query
  case DXIL::OpCode::HitObject_FromRayQuery:
  case DXIL::OpCode::HitObject_FromRayQueryWithAttrs: {
    for (unsigned i = 1; i < CI->getNumOperands(); ++i) {
      Value *Arg = CI->getArgOperand(i);
      if (isa<UndefValue>(Arg))
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
    }
    break;
  }

  case DXIL::OpCode::HitObject_Invoke: {
    if (isa<UndefValue>(CI->getArgOperand(1)))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrUndefHitObject);
    if (isa<UndefValue>(CI->getArgOperand(2)))
      ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
  } break;
  case DXIL::OpCode::HitObject_TraceRay: {
    Value *Hdl = CI->getArgOperand(
        DxilInst_HitObject_TraceRay::arg_accelerationStructure);
    ValidateASHandle(CI, Hdl, ValCtx);
    for (unsigned ArgIdx = 2; ArgIdx < CI->getNumArgOperands(); ++ArgIdx)
      if (isa<UndefValue>(CI->getArgOperand(ArgIdx)))
        ValCtx.EmitInstrError(CI, ValidationRule::InstrNoReadingUninitialized);
    DxilInst_HitObject_TraceRay HOTraceRay(CI);
  } break;
  case DXIL::OpCode::AtomicBinOp:
  case DXIL::OpCode::AtomicCompareExchange: {
    Type *pOverloadType = OP::GetOverloadType(Opcode, CI->getCalledFunction());
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
    if (ShaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (NodeLaunchType == DXIL::NodeLaunchType::Broadcasting)
      break;

    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrSVConflictingLaunchMode,
        {"ThreadId", "SV_DispatchThreadID", GetLaunchTypeStr(NodeLaunchType)});
    break;

  case DXIL::OpCode::GroupId: // SV_GroupId
    if (ShaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (NodeLaunchType == DXIL::NodeLaunchType::Broadcasting)
      break;

    ValCtx.EmitInstrFormatError(
        CI, ValidationRule::InstrSVConflictingLaunchMode,
        {"GroupId", "SV_GroupId", GetLaunchTypeStr(NodeLaunchType)});
    break;

  case DXIL::OpCode::ThreadIdInGroup: // SV_GroupThreadID
    if (ShaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (NodeLaunchType == DXIL::NodeLaunchType::Broadcasting ||
        NodeLaunchType == DXIL::NodeLaunchType::Coalescing)
      break;

    ValCtx.EmitInstrFormatError(CI,
                                ValidationRule::InstrSVConflictingLaunchMode,
                                {"ThreadIdInGroup", "SV_GroupThreadID",
                                 GetLaunchTypeStr(NodeLaunchType)});

    break;

  case DXIL::OpCode::FlattenedThreadIdInGroup: // SV_GroupIndex
    if (ShaderKind != DXIL::ShaderKind::Node) {
      break;
    }

    if (NodeLaunchType == DXIL::NodeLaunchType::Broadcasting ||
        NodeLaunchType == DXIL::NodeLaunchType::Coalescing)
      break;

    ValCtx.EmitInstrFormatError(CI,
                                ValidationRule::InstrSVConflictingLaunchMode,
                                {"FlattenedThreadIdInGroup", "SV_GroupIndex",
                                 GetLaunchTypeStr(NodeLaunchType)});

    break;
  case DXIL::OpCode::MatVecMul:
  case DXIL::OpCode::MatVecMulAdd:
    ValidateImmOperandsForMatVecOps(CI, Opcode, ValCtx);
    break;
  case DXIL::OpCode::OuterProductAccumulate:
    ValidateImmOperandsForOuterProdAcc(CI, ValCtx);
    break;
  case DXIL::OpCode::VectorAccumulate:

    break;

  default:
    // TODO: make sure every Opcode is checked.
    // Skip opcodes don't need special check.
    break;
  }
}

static bool IsDxilFunction(llvm::Function *F) {
  unsigned ArgSize = F->arg_size();
  if (ArgSize < 1) {
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
  OP *HlslOP = ValCtx.DxilMod.GetOP();
  bool IsDxilOp = OP::IsDxilOpFunc(F);
  Type *VoidTy = Type::getVoidTy(F->getContext());

  for (User *user : F->users()) {
    CallInst *CI = dyn_cast<CallInst>(user);
    if (!CI) {
      ValCtx.EmitFnFormatError(F, ValidationRule::DeclFnIsCalled,
                               {F->getName()});
      continue;
    }

    // Skip call to external user defined function
    if (!IsDxilOp)
      continue;

    Value *ArgOpcode = CI->getArgOperand(0);
    ConstantInt *ConstOpcode = dyn_cast<ConstantInt>(ArgOpcode);
    if (!ConstOpcode) {
      // Opcode not immediate; function body will validate this error.
      continue;
    }

    unsigned Opcode = ConstOpcode->getLimitedValue();
    if (Opcode >= (unsigned)DXIL::OpCode::NumOpCodes) {
      // invalid Opcode; function body will validate this error.
      continue;
    }

    DXIL::OpCode DxilOpcode = (DXIL::OpCode)Opcode;

    // In some cases, no overloads are provided (void is exclusive to others)
    Function *DxilFunc;
    if (HlslOP->IsOverloadLegal(DxilOpcode, VoidTy)) {
      DxilFunc = HlslOP->GetOpFunc(DxilOpcode, VoidTy);
    } else {
      Type *Ty = OP::GetOverloadType(DxilOpcode, CI->getCalledFunction());
      try {
        if (!HlslOP->IsOverloadLegal(DxilOpcode, Ty)) {
          ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
          continue;
        }
      } catch (...) {
        ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
        continue;
      }
      DxilFunc = HlslOP->GetOpFunc(DxilOpcode, Ty);
    }

    if (!DxilFunc) {
      // Cannot find DxilFunction based on Opcode and type.
      ValCtx.EmitInstrError(CI, ValidationRule::InstrOload);
      continue;
    }

    if (DxilFunc->getFunctionType() != F->getFunctionType()) {
      ValCtx.EmitInstrFormatError(CI, ValidationRule::InstrCallOload,
                                  {DxilFunc->getName()});
      continue;
    }

    unsigned major = pSM->GetMajor();
    unsigned minor = pSM->GetMinor();
    if (ValCtx.isLibProfile) {
      Function *CallingFunction = CI->getParent()->getParent();
      DXIL::ShaderKind SK = DXIL::ShaderKind::Library;
      if (ValCtx.DxilMod.HasDxilFunctionProps(CallingFunction))
        SK = ValCtx.DxilMod.GetDxilFunctionProps(CallingFunction).shaderKind;
      else if (ValCtx.DxilMod.IsPatchConstantShader(CallingFunction))
        SK = DXIL::ShaderKind::Hull;
      if (!ValidateOpcodeInProfile(DxilOpcode, SK, major, minor)) {
        // Opcode not available in profile.
        // produces: "lib_6_3(ps)", or "lib_6_3(anyhit)" for shader types
        // Or: "lib_6_3(lib)" for library function
        std::string ShaderModel = pSM->GetName();
        ShaderModel += std::string("(") + ShaderModel::GetKindName(SK) + ")";
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::SmOpcode,
            {HlslOP->GetOpCodeName(DxilOpcode), ShaderModel});
        continue;
      }
    } else {
      if (!ValidateOpcodeInProfile(DxilOpcode, pSM->GetKind(), major, minor)) {
        // Opcode not available in profile.
        ValCtx.EmitInstrFormatError(
            CI, ValidationRule::SmOpcode,
            {HlslOP->GetOpCodeName(DxilOpcode), pSM->GetName()});
        continue;
      }
    }

    // Check more detail.
    ValidateDxilOperationCallInProfile(CI, DxilOpcode, pSM, ValCtx);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Instruction validation functions.                                         //

static bool IsDxilBuiltinStructType(StructType *ST, hlsl::OP *HlslOP) {
  if (ST == HlslOP->GetBinaryWithCarryType())
    return true;
  if (ST == HlslOP->GetBinaryWithTwoOutputsType())
    return true;
  if (ST == HlslOP->GetFourI32Type())
    return true;
  if (ST == HlslOP->GetFourI16Type())
    return true;
  if (ST == HlslOP->GetDimensionsType())
    return true;
  if (ST == HlslOP->GetHandleType())
    return true;
  if (ST == HlslOP->GetSamplePosType())
    return true;
  if (ST == HlslOP->GetSplitDoubleType())
    return true;

  unsigned EltNum = ST->getNumElements();
  Type *EltTy = ST->getElementType(0);
  switch (EltNum) {
  case 2:
    // Check if it's a native vector resret.
    if (EltTy->isVectorTy())
      return ST == HlslOP->GetResRetType(EltTy);
    LLVM_FALLTHROUGH;
  case 4:
  case 8: // 2 for doubles, 8 for halfs.
    return ST == HlslOP->GetCBufferRetType(EltTy);
    break;
  case 5:
    return ST == HlslOP->GetResRetType(EltTy);
    break;
  default:
    return false;
  }
}

// outer type may be: [ptr to][1 dim array of]( UDT struct | scalar )
// inner type (UDT struct member) may be: [N dim array of]( UDT struct | scalar
// ) scalar type may be: ( float(16|32|64) | int(16|32|64) )
static bool ValidateType(Type *Ty, ValidationContext &ValCtx,
                         bool IsInner = false) {
  DXASSERT_NOMSG(Ty != nullptr);
  if (Ty->isPointerTy()) {
    Type *EltTy = Ty->getPointerElementType();
    if (IsInner || EltTy->isPointerTy()) {
      ValCtx.EmitTypeError(Ty, ValidationRule::TypesNoPtrToPtr);
      return false;
    }
    Ty = EltTy;
  }
  if (Ty->isArrayTy()) {
    Type *EltTy = Ty->getArrayElementType();
    if (!IsInner && isa<ArrayType>(EltTy)) {
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
    bool Result = true;
    StructType *ST = cast<StructType>(Ty);

    StringRef Name = ST->getName();
    if (Name.startswith("dx.")) {
      // Allow handle type.
      if (ValCtx.HandleTy == Ty)
        return true;
      hlsl::OP *HlslOP = ValCtx.DxilMod.GetOP();
      // Allow HitObject type.
      if (ST == HlslOP->GetHitObjectType())
        return true;
      if (IsDxilBuiltinStructType(ST, HlslOP)) {
        ValCtx.EmitTypeError(Ty, ValidationRule::InstrDxilStructUser);
        Result = false;
      }

      ValCtx.EmitTypeError(Ty, ValidationRule::DeclDxilNsReserved);
      Result = false;
    }
    for (auto e : ST->elements()) {
      if (!ValidateType(e, ValCtx, /*IsInner*/ true)) {
        Result = false;
      }
    }
    return Result;
  }
  if (Ty->isFloatTy() || Ty->isHalfTy() || Ty->isDoubleTy()) {
    return true;
  }
  if (Ty->isIntegerTy()) {
    unsigned Width = Ty->getIntegerBitWidth();
    if (Width != 1 && Width != 8 && Width != 16 && Width != 32 && Width != 64) {
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
                                unsigned Index, uint64_t *PValue) {
  *PValue = 0;
  if (pMD->getNumOperands() < Index) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  ConstantAsMetadata *C = dyn_cast<ConstantAsMetadata>(pMD->getOperand(Index));
  if (C == nullptr) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  ConstantInt *CI = dyn_cast<ConstantInt>(C->getValue());
  if (CI == nullptr) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
    return false;
  }
  *PValue = CI->getValue().getZExtValue();
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

  uint64_t Val;
  if (!GetNodeOperandAsInt(ValCtx, pMD, 0, &Val)) {
    return false;
  }
  if (Val == 1) {
    return true;
  }
  if (Val != 0) {
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
                                 CallInst *SetMeshOutputCounts,
                                 CallInst *GetMeshPayload) {
  if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
    DXIL::ShaderKind ShaderKind =
        ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
    if (ShaderKind != DXIL::ShaderKind::Mesh)
      return;
  } else {
    return;
  }

  DominatorTreeAnalysis DTA;
  DominatorTree DT = DTA.run(*F);

  for (auto B = F->begin(), BEnd = F->end(); B != BEnd; ++B) {
    bool FoundSetMeshOutputCountsInCurrentBb = false;
    for (auto It = B->begin(), ItEnd = B->end(); It != ItEnd; ++It) {
      llvm::Instruction &I = *It;

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

          if (CI == SetMeshOutputCounts) {
            FoundSetMeshOutputCountsInCurrentBb = true;
          }
          Value *OpcodeVal = CI->getOperand(0);
          ConstantInt *OpcodeConst = dyn_cast<ConstantInt>(OpcodeVal);
          unsigned Opcode = OpcodeConst->getLimitedValue();
          DXIL::OpCode DxilOpcode = (DXIL::OpCode)Opcode;

          if (DxilOpcode == DXIL::OpCode::StoreVertexOutput ||
              DxilOpcode == DXIL::OpCode::StorePrimitiveOutput ||
              DxilOpcode == DXIL::OpCode::EmitIndices) {
            if (SetMeshOutputCounts == nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMissingSetMeshOutputCounts);
            } else if (!FoundSetMeshOutputCountsInCurrentBb &&
                       !DT.dominates(SetMeshOutputCounts->getParent(),
                                     I.getParent())) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrNonDominatingSetMeshOutputCounts);
            }
          }
        }
      }
    }
  }

  if (GetMeshPayload) {
    PointerType *PayloadPTy = cast<PointerType>(GetMeshPayload->getType());
    StructType *PayloadTy =
        cast<StructType>(PayloadPTy->getPointerElementType());
    const DataLayout &DL = F->getParent()->getDataLayout();
    unsigned PayloadSize = DL.getTypeAllocSize(PayloadTy);

    DxilFunctionProps &Prop = ValCtx.DxilMod.GetDxilFunctionProps(F);

    if (Prop.ShaderProps.MS.payloadSizeInBytes < PayloadSize) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderPayloadSizeDeclared,
          {F->getName(), std::to_string(PayloadSize),
           std::to_string(Prop.ShaderProps.MS.payloadSizeInBytes)});
    }

    if (Prop.ShaderProps.MS.payloadSizeInBytes > DXIL::kMaxMSASPayloadBytes) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderPayloadSize,
          {F->getName(), std::to_string(Prop.ShaderProps.MS.payloadSizeInBytes),
           std::to_string(DXIL::kMaxMSASPayloadBytes)});
    }
  }
}

static void ValidateAsIntrinsics(Function *F, ValidationContext &ValCtx,
                                 CallInst *DispatchMesh) {
  if (ValCtx.DxilMod.HasDxilFunctionProps(F)) {
    DXIL::ShaderKind ShaderKind =
        ValCtx.DxilMod.GetDxilFunctionProps(F).shaderKind;
    if (ShaderKind != DXIL::ShaderKind::Amplification)
      return;

    if (DispatchMesh) {
      DxilInst_DispatchMesh DispatchMeshCall(DispatchMesh);
      Value *OperandVal = DispatchMeshCall.get_payload();
      Type *PayloadTy = OperandVal->getType();
      const DataLayout &DL = F->getParent()->getDataLayout();
      unsigned PayloadSize = DL.getTypeAllocSize(PayloadTy);

      DxilFunctionProps &Prop = ValCtx.DxilMod.GetDxilFunctionProps(F);

      if (Prop.ShaderProps.AS.payloadSizeInBytes < PayloadSize) {
        ValCtx.EmitInstrFormatError(
            DispatchMesh,
            ValidationRule::SmAmplificationShaderPayloadSizeDeclared,
            {F->getName(), std::to_string(PayloadSize),
             std::to_string(Prop.ShaderProps.AS.payloadSizeInBytes)});
      }

      if (Prop.ShaderProps.AS.payloadSizeInBytes > DXIL::kMaxMSASPayloadBytes) {
        ValCtx.EmitInstrFormatError(
            DispatchMesh, ValidationRule::SmAmplificationShaderPayloadSize,
            {F->getName(),
             std::to_string(Prop.ShaderProps.AS.payloadSizeInBytes),
             std::to_string(DXIL::kMaxMSASPayloadBytes)});
      }
    }

  } else {
    return;
  }

  if (DispatchMesh == nullptr) {
    ValCtx.EmitFnError(F, ValidationRule::InstrNotOnceDispatchMesh);
    return;
  }

  PostDominatorTree PDT;
  PDT.runOnFunction(*F);

  if (!PDT.dominates(DispatchMesh->getParent(), &F->getEntryBlock())) {
    ValCtx.EmitInstrError(DispatchMesh,
                          ValidationRule::InstrNonDominatingDispatchMesh);
  }

  Function *DispatchMeshFunc = DispatchMesh->getCalledFunction();
  FunctionType *DispatchMeshFuncTy = DispatchMeshFunc->getFunctionType();
  PointerType *PayloadPTy =
      cast<PointerType>(DispatchMeshFuncTy->getParamType(4));
  StructType *PayloadTy = cast<StructType>(PayloadPTy->getPointerElementType());
  const DataLayout &DL = F->getParent()->getDataLayout();
  unsigned PayloadSize = DL.getTypeAllocSize(PayloadTy);

  if (PayloadSize > DXIL::kMaxMSASPayloadBytes) {
    ValCtx.EmitInstrFormatError(
        DispatchMesh, ValidationRule::SmAmplificationShaderPayloadSize,
        {F->getName(), std::to_string(PayloadSize),
         std::to_string(DXIL::kMaxMSASPayloadBytes)});
  }
}

static void ValidateControlFlowHint(BasicBlock &BB, ValidationContext &ValCtx) {
  // Validate controlflow hint.
  TerminatorInst *TI = BB.getTerminator();
  if (!TI)
    return;

  MDNode *pNode = TI->getMetadata(DxilMDHelper::kDxilControlFlowHintMDName);
  if (!pNode)
    return;

  if (pNode->getNumOperands() < 3)
    return;

  bool HasBranch = false;
  bool HasFlatten = false;
  bool ForceCase = false;

  for (unsigned I = 2; I < pNode->getNumOperands(); I++) {
    uint64_t Value = 0;
    if (GetNodeOperandAsInt(ValCtx, pNode, I, &Value)) {
      DXIL::ControlFlowHint Hint = static_cast<DXIL::ControlFlowHint>(Value);
      switch (Hint) {
      case DXIL::ControlFlowHint::Flatten:
        HasFlatten = true;
        break;
      case DXIL::ControlFlowHint::Branch:
        HasBranch = true;
        break;
      case DXIL::ControlFlowHint::ForceCase:
        ForceCase = true;
        break;
      default:
        ValCtx.EmitMetaError(pNode, ValidationRule::MetaInvalidControlFlowHint);
      }
    }
  }
  if (HasBranch && HasFlatten) {
    ValCtx.EmitMetaError(pNode, ValidationRule::MetaBranchFlatten);
  }
  if (ForceCase && !isa<SwitchInst>(TI)) {
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
    MDNode *RootNode = dyn_cast<MDNode>(Node->getOperand(1));
    if (!RootNode) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ValidateTBAAMetadata(RootNode, ValCtx);
    }
  } break;
  case 3: {
    MDNode *RootNode = dyn_cast<MDNode>(Node->getOperand(1));
    if (!RootNode) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ValidateTBAAMetadata(RootNode, ValCtx);
    }
    ConstantAsMetadata *PointsToConstMem =
        dyn_cast<ConstantAsMetadata>(Node->getOperand(2));
    if (!PointsToConstMem) {
      ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
    } else {
      ConstantInt *IsConst =
          dyn_cast<ConstantInt>(PointsToConstMem->getValue());
      if (!IsConst) {
        ValCtx.EmitMetaError(Node, ValidationRule::MetaWellFormed);
      } else if (IsConst->getValue().getLimitedValue() > 1) {
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
  uint64_t Val;
  if (!GetNodeOperandAsInt(ValCtx, pMD, 0, &Val)) {
    ValCtx.EmitMetaError(pMD, ValidationRule::MetaWellFormed);
  }
  if (Val != 1) {
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
  AttributeSet AttrSet = F->getAttributes().getFnAttributes();
  // fp32-denorm-mode
  if (AttrSet.hasAttribute(AttributeSet::FunctionIndex,
                           DXIL::kFP32DenormKindString)) {
    Attribute Attr = AttrSet.getAttribute(AttributeSet::FunctionIndex,
                                          DXIL::kFP32DenormKindString);
    StringRef StrValue = Attr.getValueAsString();
    if (!StrValue.equals(DXIL::kFP32DenormValueAnyString) &&
        !StrValue.equals(DXIL::kFP32DenormValueFtzString) &&
        !StrValue.equals(DXIL::kFP32DenormValuePreserveString)) {
      ValCtx.EmitFnAttributeError(F, Attr.getKindAsString(),
                                  Attr.getValueAsString());
    }
  }
  // TODO: If validating libraries, we should remove all unknown function
  // attributes. For each attribute, check if it is a known attribute
  for (unsigned I = 0, E = AttrSet.getNumSlots(); I != E; ++I) {
    for (auto AttrIter = AttrSet.begin(I), AttrEnd = AttrSet.end(I);
         AttrIter != AttrEnd; ++AttrIter) {
      if (!AttrIter->isStringAttribute()) {
        continue;
      }
      StringRef Kind = AttrIter->getKindAsString();
      if (!Kind.equals(DXIL::kFP32DenormKindString) &&
          !Kind.equals(DXIL::kWaveOpsIncludeHelperLanesString)) {
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
  unsigned Opcode = I.getOpcode();
  if (Opcode == Instruction::InsertElement ||
      Opcode == Instruction::ExtractElement ||
      Opcode == Instruction::ShuffleVector)
    return false;

  return true;
}

static void ValidateFunctionBody(Function *F, ValidationContext &ValCtx) {
  bool SupportsMinPrecision =
      ValCtx.DxilMod.GetGlobalFlags() & DXIL::kEnableMinPrecision;
  bool SupportsLifetimeIntrinsics =
      ValCtx.DxilMod.GetShaderModel()->IsSM66Plus();
  SmallVector<CallInst *, 16> GradientOps;
  SmallVector<CallInst *, 16> Barriers;
  CallInst *SetMeshOutputCounts = nullptr;
  CallInst *GetMeshPayload = nullptr;
  CallInst *DispatchMesh = nullptr;
  hlsl::OP *HlslOP = ValCtx.DxilMod.GetOP();

  for (auto B = F->begin(), BEnd = F->end(); B != BEnd; ++B) {
    for (auto It = B->begin(), ItEnd = B->end(); It != ItEnd; ++It) {
      llvm::Instruction &I = *It;

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

          Value *OpcodeVal = CI->getOperand(0);
          ConstantInt *OpcodeConst = dyn_cast<ConstantInt>(OpcodeVal);
          if (OpcodeConst == nullptr) {
            ValCtx.EmitInstrFormatError(&I, ValidationRule::InstrOpConst,
                                        {"Opcode", "DXIL operation"});
            continue;
          }

          unsigned Opcode = OpcodeConst->getLimitedValue();
          if (Opcode >= static_cast<unsigned>(DXIL::OpCode::NumOpCodes)) {
            ValCtx.EmitInstrFormatError(
                &I, ValidationRule::InstrIllegalDXILOpCode,
                {std::to_string((unsigned)DXIL::OpCode::NumOpCodes),
                 std::to_string(Opcode)});
            continue;
          }
          DXIL::OpCode DxilOpcode = (DXIL::OpCode)Opcode;

          bool IllegalOpFunc = true;
          for (auto &It : HlslOP->GetOpFuncList(DxilOpcode)) {
            if (It.second == FCalled) {
              IllegalOpFunc = false;
              break;
            }
          }

          if (IllegalOpFunc) {
            ValCtx.EmitInstrFormatError(
                &I, ValidationRule::InstrIllegalDXILOpFunction,
                {FCalled->getName(), OP::GetOpCodeName(DxilOpcode)});
            continue;
          }

          if (OP::IsDxilOpGradient(DxilOpcode)) {
            GradientOps.push_back(CI);
          }

          if (DxilOpcode == DXIL::OpCode::Barrier) {
            Barriers.push_back(CI);
          }
          // External function validation will check the parameter
          // list. This function will check that the call does not
          // violate any rules.

          if (DxilOpcode == DXIL::OpCode::SetMeshOutputCounts) {
            // validate the call count of SetMeshOutputCounts
            if (SetMeshOutputCounts != nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMultipleSetMeshOutputCounts);
            }
            SetMeshOutputCounts = CI;
          }

          if (DxilOpcode == DXIL::OpCode::GetMeshPayload) {
            // validate the call count of GetMeshPayload
            if (GetMeshPayload != nullptr) {
              ValCtx.EmitInstrError(
                  &I, ValidationRule::InstrMultipleGetMeshPayload);
            }
            GetMeshPayload = CI;
          }

          if (DxilOpcode == DXIL::OpCode::DispatchMesh) {
            // validate the call count of DispatchMesh
            if (DispatchMesh != nullptr) {
              ValCtx.EmitInstrError(&I,
                                    ValidationRule::InstrNotOnceDispatchMesh);
            }
            DispatchMesh = CI;
          }
        }
        continue;
      }

      for (Value *op : I.operands()) {
        if (isa<UndefValue>(op)) {
          bool LegalUndef = isa<PHINode>(&I);
          if (isa<InsertElementInst>(&I)) {
            LegalUndef = op == I.getOperand(0);
          }
          if (isa<ShuffleVectorInst>(&I)) {
            LegalUndef = op == I.getOperand(1);
          }
          if (isa<StoreInst>(&I)) {
            LegalUndef = op == I.getOperand(0);
          }

          if (!LegalUndef)
            ValCtx.EmitInstrError(&I,
                                  ValidationRule::InstrNoReadingUninitialized);
        } else if (ConstantExpr *CE = dyn_cast<ConstantExpr>(op)) {
          for (Value *OpCE : CE->operands()) {
            if (isa<UndefValue>(OpCE)) {
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

      unsigned Opcode = I.getOpcode();
      switch (Opcode) {
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
        bool AllImmIndex = true;
        for (auto Idx = GEP->idx_begin(), E = GEP->idx_end(); Idx != E; Idx++) {
          if (!isa<ConstantInt>(Idx)) {
            AllImmIndex = false;
            break;
          }
        }
        if (AllImmIndex) {
          const DataLayout &DL = ValCtx.DL;

          Value *Ptr = GEP->getPointerOperand();
          unsigned Size =
              DL.getTypeAllocSize(Ptr->getType()->getPointerElementType());
          unsigned ValSize =
              DL.getTypeAllocSize(GEP->getType()->getPointerElementType());

          SmallVector<Value *, 8> Indices(GEP->idx_begin(), GEP->idx_end());
          unsigned Offset =
              DL.getIndexedOffset(GEP->getPointerOperandType(), Indices);
          if ((Offset + ValSize) > Size) {
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
        PointerType *PtrType = cast<PointerType>(Ptr->getType());
        Type *ElType = PtrType->getElementType();
        const ShaderModel *pSM = ValCtx.DxilMod.GetShaderModel();
        if ((ElType->isIntegerTy(64)) && !pSM->IsSM66Plus())
          ValCtx.EmitInstrFormatError(
              &I, ValidationRule::SmOpcodeInInvalidFunction,
              {"64-bit atomic operations", "Shader Model 6.6+"});

        if (PtrType->getAddressSpace() != DXIL::kTGSMAddrSpace &&
            PtrType->getAddressSpace() != DXIL::kNodeRecordAddrSpace)
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
    ValidateControlFlowHint(*B, ValCtx);
  }

  ValidateMsIntrinsics(F, ValCtx, SetMeshOutputCounts, GetMeshPayload);

  ValidateAsIntrinsics(F, ValCtx, DispatchMesh);
}

static void ValidateNodeInputRecord(Function *F, ValidationContext &ValCtx) {
  // if there are no function props or LaunchType is Invalid, there is nothing
  // to do here
  if (!ValCtx.DxilMod.HasDxilFunctionProps(F))
    return;
  auto &Props = ValCtx.DxilMod.GetDxilFunctionProps(F);
  if (!Props.IsNode())
    return;
  if (Props.InputNodes.size() > 1) {
    ValCtx.EmitFnFormatError(
        F, ValidationRule::DeclMultipleNodeInputs,
        {F->getName(), std::to_string(Props.InputNodes.size())});
  }
  for (auto &input : Props.InputNodes) {
    if (!input.Flags.RecordTypeMatchesLaunchType(Props.Node.LaunchType)) {
      // We allow EmptyNodeInput here, as that may have been added implicitly
      // if there was no input specified
      if (input.Flags.IsEmptyInput())
        continue;

      llvm::StringRef ValidInputs = "";
      switch (Props.Node.LaunchType) {
      case DXIL::NodeLaunchType::Broadcasting:
        ValidInputs = "{RW}DispatchNodeInputRecord";
        break;
      case DXIL::NodeLaunchType::Coalescing:
        ValidInputs = "{RW}GroupNodeInputRecords or EmptyNodeInput";
        break;
      case DXIL::NodeLaunchType::Thread:
        ValidInputs = "{RW}ThreadNodeInputRecord";
        break;
      default:
        llvm_unreachable("invalid launch type");
      }
      ValCtx.EmitFnFormatError(
          F, ValidationRule::DeclNodeLaunchInputType,
          {ShaderModel::GetNodeLaunchTypeName(Props.Node.LaunchType),
           F->getName(), ValidInputs});
    }
  }
}

static void ValidateFunction(Function &F, ValidationContext &ValCtx) {
  if (F.isDeclaration()) {
    ValidateExternalFunction(&F, ValCtx);
    if (F.isIntrinsic() || IsDxilFunction(&F))
      return;
  } else {
    DXIL::ShaderKind ShaderKind = DXIL::ShaderKind::Library;
    bool IsShader = ValCtx.DxilMod.HasDxilFunctionProps(&F);
    unsigned NumUDTShaderArgs = 0;
    if (IsShader) {
      ShaderKind = ValCtx.DxilMod.GetDxilFunctionProps(&F).shaderKind;
      switch (ShaderKind) {
      case DXIL::ShaderKind::AnyHit:
      case DXIL::ShaderKind::ClosestHit:
        NumUDTShaderArgs = 2;
        break;
      case DXIL::ShaderKind::Miss:
      case DXIL::ShaderKind::Callable:
        NumUDTShaderArgs = 1;
        break;
      case DXIL::ShaderKind::Compute: {
        DxilModule &DM = ValCtx.DxilMod;
        if (DM.HasDxilEntryProps(&F)) {
          DxilEntryProps &EntryProps = DM.GetDxilEntryProps(&F);
          // Check that compute has no node metadata
          if (EntryProps.props.IsNode()) {
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
      IsShader = ValCtx.DxilMod.IsPatchConstantShader(&F);
    }

    // Entry function should not have parameter.
    if (IsShader && 0 == NumUDTShaderArgs && !F.arg_empty())
      ValCtx.EmitFnFormatError(&F, ValidationRule::FlowFunctionCall,
                               {F.getName()});

    // Shader functions should return void.
    if (IsShader && !F.getReturnType()->isVoidTy())
      ValCtx.EmitFnFormatError(&F, ValidationRule::DeclShaderReturnVoid,
                               {F.getName()});

    auto ArgFormatError = [&](Function &F, Argument &Arg, ValidationRule Rule) {
      if (Arg.hasName())
        ValCtx.EmitFnFormatError(&F, Rule, {Arg.getName().str(), F.getName()});
      else
        ValCtx.EmitFnFormatError(&F, Rule,
                                 {std::to_string(Arg.getArgNo()), F.getName()});
    };

    unsigned NumArgs = 0;
    for (auto &Arg : F.args()) {
      Type *ArgTy = Arg.getType();
      if (ArgTy->isPointerTy())
        ArgTy = ArgTy->getPointerElementType();

      NumArgs++;
      if (NumUDTShaderArgs) {
        if (Arg.getArgNo() >= NumUDTShaderArgs) {
          ArgFormatError(F, Arg, ValidationRule::DeclExtraArgs);
        } else if (!ArgTy->isStructTy()) {
          switch (ShaderKind) {
          case DXIL::ShaderKind::Callable:
            ArgFormatError(F, Arg, ValidationRule::DeclParamStruct);
            break;
          default:
            ArgFormatError(F, Arg,
                           Arg.getArgNo() == 0
                               ? ValidationRule::DeclPayloadStruct
                               : ValidationRule::DeclAttrStruct);
          }
        }
        continue;
      }

      while (ArgTy->isArrayTy()) {
        ArgTy = ArgTy->getArrayElementType();
      }

      if (ArgTy->isStructTy() && !ValCtx.isLibProfile) {
        ArgFormatError(F, Arg, ValidationRule::DeclFnFlattenParam);
        break;
      }
    }

    if (NumArgs < NumUDTShaderArgs && ShaderKind != DXIL::ShaderKind::Node) {
      StringRef ArgType[2] = {
          ShaderKind == DXIL::ShaderKind::Callable ? "params" : "payload",
          "attributes"};
      for (unsigned I = NumArgs; I < NumUDTShaderArgs; I++) {
        ValCtx.EmitFnFormatError(
            &F, ValidationRule::DeclShaderMissingArg,
            {ShaderModel::GetKindName(ShaderKind), F.getName(), ArgType[I]});
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
  bool IsInternalGv =
      dxilutil::IsStaticGlobal(&GV) || dxilutil::IsSharedMemoryGlobal(&GV);

  if (ValCtx.isLibProfile) {
    auto IsCBufferGlobal =
        [&](const std::vector<std::unique_ptr<DxilCBuffer>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };
    auto IsResourceGlobal =
        [&](const std::vector<std::unique_ptr<DxilResource>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };
    auto IsSamplerGlobal =
        [&](const std::vector<std::unique_ptr<DxilSampler>> &ResTab) -> bool {
      for (auto &Res : ResTab)
        if (Res->GetGlobalSymbol() == &GV)
          return true;
      return false;
    };

    bool IsRes = IsCBufferGlobal(ValCtx.DxilMod.GetCBuffers());
    IsRes |= IsResourceGlobal(ValCtx.DxilMod.GetUAVs());
    IsRes |= IsResourceGlobal(ValCtx.DxilMod.GetSRVs());
    IsRes |= IsSamplerGlobal(ValCtx.DxilMod.GetSamplers());
    IsInternalGv |= IsRes;

    // Allow special dx.ishelper for library target
    if (GV.getName().compare(DXIL::kDxIsHelperGlobalName) == 0) {
      Type *Ty = GV.getType()->getPointerElementType();
      if (Ty->isIntegerTy() && Ty->getScalarSizeInBits() == 32) {
        IsInternalGv = true;
      }
    }
  }

  if (!IsInternalGv) {
    if (!GV.user_empty()) {
      bool HasInstructionUser = false;
      for (User *U : GV.users()) {
        if (isa<Instruction>(U)) {
          HasInstructionUser = true;
          break;
        }
      }
      // External GV should not have instruction user.
      if (HasInstructionUser) {
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
                                    std::vector<StoreInst *> &FixAddrTGSMList) {
  for (User *U : V->users()) {
    if (GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
      if (isa<ConstantExpr>(GEP) || GEP->hasAllConstantIndices()) {
        CollectFixAddressAccess(GEP, FixAddrTGSMList);
      }
    } else if (StoreInst *SI = dyn_cast<StoreInst>(U)) {
      FixAddrTGSMList.emplace_back(SI);
    }
  }
}

static bool IsDivergent(Value *V) {
  // TODO: return correct result.
  return false;
}

static void ValidateTGSMRaceCondition(std::vector<StoreInst *> &FixAddrTGSMList,
                                      ValidationContext &ValCtx) {
  std::unordered_set<Function *> FixAddrTGSMFuncSet;
  for (StoreInst *I : FixAddrTGSMList) {
    BasicBlock *BB = I->getParent();
    FixAddrTGSMFuncSet.insert(BB->getParent());
  }

  for (auto &F : ValCtx.DxilMod.GetModule()->functions()) {
    if (F.isDeclaration() || !FixAddrTGSMFuncSet.count(&F))
      continue;

    PostDominatorTree PDT;
    PDT.runOnFunction(F);

    BasicBlock *Entry = &F.getEntryBlock();

    for (StoreInst *SI : FixAddrTGSMList) {
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
  std::vector<StoreInst *> FixAddrTGSMList;
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
              DxilFunctionProps &Props = M.GetDxilEntryProps(F).props;
              if (!Props.IsCS() && !Props.IsAS() && !Props.IsMS() &&
                  !Props.IsNode()) {
                ValCtx.EmitInstrFormatError(I,
                                            ValidationRule::SmTGSMUnsupported,
                                            {"from non-compute entry points"});
              }
            }
          }
        }
      }
      TGSMSize += DL.getTypeAllocSize(GV.getType()->getElementType());
      CollectFixAddressAccess(&GV, FixAddrTGSMList);
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

  if (!FixAddrTGSMList.empty()) {
    ValidateTGSMRaceCondition(FixAddrTGSMList, ValCtx);
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
      uint64_t MajorVer, MinorVer;
      if (GetNodeOperandAsInt(ValCtx, pVerValues, 0, &MajorVer) &&
          GetNodeOperandAsInt(ValCtx, pVerValues, 1, &MinorVer)) {
        unsigned CurMajor, CurMinor;
        GetValidationVersion(&CurMajor, &CurMinor);
        // This will need to be updated as major/minor versions evolve,
        // depending on the degree of compat across versions.
        if (MajorVer == CurMajor && MinorVer <= CurMinor) {
          return;
        } else {
          ValCtx.EmitFormatError(
              ValidationRule::MetaVersionSupported,
              {"Validator", std::to_string(MajorVer), std::to_string(MinorVer),
               std::to_string(CurMajor), std::to_string(CurMinor)});
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
      uint64_t MajorVer, MinorVer;
      if (GetNodeOperandAsInt(ValCtx, pVerValues, 0, &MajorVer) &&
          GetNodeOperandAsInt(ValCtx, pVerValues, 1, &MinorVer)) {
        // This will need to be updated as dxil major/minor versions evolve,
        // depending on the degree of compat across versions.
        if ((MajorVer == DXIL::kDxilMajor && MinorVer <= DXIL::kDxilMinor) &&
            (MajorVer == ValCtx.m_DxilMajor &&
             MinorVer == ValCtx.m_DxilMinor)) {
          return;
        } else {
          ValCtx.EmitFormatError(ValidationRule::MetaVersionSupported,
                                 {"Dxil", std::to_string(MajorVer),
                                  std::to_string(MinorVer),
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
    for (unsigned I = 0, End = TA->getNumOperands(); I < End; ++I) {
      MDTuple *TANode = dyn_cast<MDTuple>(TA->getOperand(I));
      if (TANode->getNumOperands() < 3) {
        ValCtx.EmitMetaError(TANode, ValidationRule::MetaWellFormed);
        return;
      }
      ConstantInt *Tag = mdconst::extract<ConstantInt>(TANode->getOperand(0));
      uint64_t TagValue = Tag->getZExtValue();
      if (TagValue != DxilMDHelper::kDxilTypeSystemStructTag &&
          TagValue != DxilMDHelper::kDxilTypeSystemFunctionTag) {
        ValCtx.EmitMetaError(TANode, ValidationRule::MetaWellFormed);
        return;
      }
    }
  }
}

static void ValidateBitcode(ValidationContext &ValCtx) {
  std::string DiagStr;
  raw_string_ostream DiagStream(DiagStr);
  if (llvm::verifyModule(ValCtx.M, &DiagStream)) {
    ValCtx.EmitError(ValidationRule::BitcodeValid);
    dxilutil::EmitErrorOnContext(ValCtx.M.getContext(), DiagStream.str());
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

  for (unsigned I = 0, End = EPs->getNumOperands(); I < End; ++I) {
    MDTuple *EPNodeRef = dyn_cast<MDTuple>(EPs->getOperand(I));
    if (EPNodeRef->getNumOperands() < 5) {
      ValCtx.EmitMetaError(EPNodeRef, ValidationRule::MetaWellFormed);
      return;
    }
    // get access to the digit that represents the metadata number that
    // would store entry properties
    const llvm::MDOperand &MOp =
        EPNodeRef->getOperand(EPNodeRef->getNumOperands() - 1);
    // the final operand to the entry points tuple should be a tuple.
    if (MOp == nullptr || (MOp.get())->getMetadataID() != Metadata::MDTupleKind)
      continue;

    // get access to the node that stores entry properties
    MDTuple *EPropNode = dyn_cast<MDTuple>(
        EPNodeRef->getOperand(EPNodeRef->getNumOperands() - 1));
    // find any incompatible tags inside the entry properties
    // increment j by 2 to only analyze tags, not values
    bool FoundTag = false;
    for (unsigned J = 0, End2 = EPropNode->getNumOperands(); J < End2; J += 2) {
      const MDOperand &PropertyTagOp = EPropNode->getOperand(J);
      // note, we are only looking for tags, which will be a constant
      // integer
      DXASSERT(!(PropertyTagOp == nullptr ||
                 (PropertyTagOp.get())->getMetadataID() !=
                     Metadata::ConstantAsMetadataKind),
               "tag operand should be a constant integer.");

      ConstantInt *Tag = mdconst::extract<ConstantInt>(PropertyTagOp);
      uint64_t TagValue = Tag->getZExtValue();

      // legacy wavesize is only supported between 6.6 and 6.7, so we
      // should fail if we find the ranged wave size metadata tag
      if (TagValue == DxilMDHelper::kDxilRangedWaveSizeTag) {
        // if this tag is already present in the
        // current entry point, emit an error
        if (FoundTag) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeTagDuplicate, {});
          return;
        }
        FoundTag = true;
        if (SM->IsSM66Plus() && !SM->IsSM68Plus()) {

          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeRangeNeedsSM68Plus,
                                 {});
          return;
        }
        // get the metadata that contains the
        // parameters to the wavesize attribute
        MDTuple *WaveTuple = dyn_cast<MDTuple>(EPropNode->getOperand(J + 1));
        if (WaveTuple->getNumOperands() != 3) {
          ValCtx.EmitFormatError(
              ValidationRule::SmWaveSizeRangeExpectsThreeParams, {});
          return;
        }
        for (int K = 0; K < 3; K++) {
          const MDOperand &Param = WaveTuple->getOperand(K);
          if (Param->getMetadataID() != Metadata::ConstantAsMetadataKind) {
            ValCtx.EmitFormatError(
                ValidationRule::SmWaveSizeNeedsConstantOperands, {});
            return;
          }
        }

      } else if (TagValue == DxilMDHelper::kDxilWaveSizeTag) {
        // if this tag is already present in the
        // current entry point, emit an error
        if (FoundTag) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeTagDuplicate, {});
          return;
        }
        FoundTag = true;
        MDTuple *WaveTuple = dyn_cast<MDTuple>(EPropNode->getOperand(J + 1));
        if (WaveTuple->getNumOperands() != 1) {
          ValCtx.EmitFormatError(ValidationRule::SmWaveSizeExpectsOneParam, {});
          return;
        }
        const MDOperand &Param = WaveTuple->getOperand(0);
        if (Param->getMetadataID() != Metadata::ConstantAsMetadataKind) {
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
  const std::string &Target = pModule->getTargetTriple();
  if (Target != "dxil-ms-dx") {
    ValCtx.EmitFormatError(ValidationRule::MetaTarget, {Target});
  }

  // The llvm.dbg.(cu/contents/defines/mainFileName/arg) named metadata nodes
  // are only available in debug modules, not in the validated ones.
  // llvm.bitsets is also disallowed.
  //
  // These are verified in lib/IR/Verifier.cpp.
  StringMap<bool> LlvmNamedMeta;
  LlvmNamedMeta["llvm.ident"];
  LlvmNamedMeta["llvm.module.flags"];

  for (auto &NamedMetaNode : pModule->named_metadata()) {
    if (!DxilModule::IsKnownNamedMetaData(NamedMetaNode)) {
      StringRef name = NamedMetaNode.getName();
      if (!name.startswith_lower("llvm.")) {
        ValCtx.EmitFormatError(ValidationRule::MetaKnown, {name.str()});
      } else {
        if (LlvmNamedMeta.count(name) == 0) {
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
    hlsl::DxilResourceBase &Res,
    SpacesAllocator<unsigned, DxilResourceBase> &SpaceAllocator,
    ValidationContext &ValCtx) {
  unsigned Base = Res.GetLowerBound();
  if (ValCtx.isLibProfile && !Res.IsAllocated()) {
    // Skip unallocated resource for library.
    return;
  }
  unsigned Size = Res.GetRangeSize();
  unsigned Space = Res.GetSpaceID();

  auto &Allocator = SpaceAllocator.Get(Space);
  unsigned End = Base + Size - 1;
  // unbounded
  if (End < Base)
    End = Size;
  const DxilResourceBase *ConflictRes = Allocator.Insert(&Res, Base, End);
  if (ConflictRes) {
    ValCtx.EmitFormatError(
        ValidationRule::SmResourceRangeOverlap,
        {ValCtx.GetResourceName(&Res), std::to_string(Base),
         std::to_string(Size), std::to_string(ConflictRes->GetLowerBound()),
         std::to_string(ConflictRes->GetRangeSize()), std::to_string(Space)});
  }
}

static void ValidateResource(hlsl::DxilResource &Res,
                             ValidationContext &ValCtx) {
  if (Res.IsReorderCoherent() && !ValCtx.DxilMod.GetShaderModel()->IsSM69Plus())
    ValCtx.EmitResourceError(&Res,
                             ValidationRule::InstrReorderCoherentRequiresSM69);
  switch (Res.GetKind()) {
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
    if (Res.GetSampleCount() > 0) {
      ValCtx.EmitResourceError(&Res, ValidationRule::SmSampleCountOnlyOn2DMS);
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
    if (Res.GetSamplerFeedbackType() >= DXIL::SamplerFeedbackType::LastEntry)
      ValCtx.EmitResourceError(&Res,
                               ValidationRule::SmInvalidSamplerFeedbackType);
    break;
  default:
    ValCtx.EmitResourceError(&Res, ValidationRule::SmInvalidResourceKind);
    break;
  }

  switch (Res.GetCompType().GetKind()) {
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
    if (!Res.IsStructuredBuffer() && !Res.IsRawBuffer() &&
        !Res.IsFeedbackTexture())
      ValCtx.EmitResourceError(&Res, ValidationRule::SmInvalidResourceCompType);
    break;
  }

  if (Res.IsStructuredBuffer()) {
    unsigned Stride = Res.GetElementStride();
    bool AlignedTo4Bytes = (Stride & 3) == 0;
    if (!AlignedTo4Bytes && ValCtx.M.GetDxilModule().GetUseMinPrecision()) {
      ValCtx.EmitResourceFormatError(
          &Res, ValidationRule::MetaStructBufAlignment,
          {std::to_string(4), std::to_string(Stride)});
    }
    if (Stride > DXIL::kMaxStructBufferStride) {
      ValCtx.EmitResourceFormatError(
          &Res, ValidationRule::MetaStructBufAlignmentOutOfBound,
          {std::to_string(DXIL::kMaxStructBufferStride),
           std::to_string(Stride)});
    }
  }

  if (Res.IsAnyTexture() || Res.IsTypedBuffer()) {
    Type *RetTy = Res.GetRetType();
    unsigned Size =
        ValCtx.DxilMod.GetModule()->getDataLayout().getTypeAllocSize(RetTy);
    if (Size > 4 * 4) {
      ValCtx.EmitResourceError(&Res, ValidationRule::MetaTextureType);
    }
  }
}

static void CollectCBufferRanges(
    DxilStructAnnotation *Annotation,
    SpanAllocator<unsigned, DxilFieldAnnotation> &ConstAllocator, unsigned Base,
    DxilTypeSystem &TypeSys, StringRef CbName, ValidationContext &ValCtx) {
  DXASSERT(((Base + 15) & ~(0xf)) == Base,
           "otherwise, base for struct is not aligned");
  unsigned CbSize = Annotation->GetCBufferSize();

  const StructType *ST = Annotation->GetStructType();

  for (int I = Annotation->GetNumFields() - 1; I >= 0; I--) {
    DxilFieldAnnotation &FieldAnnotation = Annotation->GetFieldAnnotation(I);
    Type *EltTy = ST->getElementType(I);

    unsigned Offset = FieldAnnotation.GetCBufferOffset();

    unsigned EltSize = dxilutil::GetLegacyCBufferFieldElementSize(
        FieldAnnotation, EltTy, TypeSys);

    bool IsOutOfBound = false;
    if (!EltTy->isAggregateType()) {
      IsOutOfBound = (Offset + EltSize) > CbSize;
      if (!IsOutOfBound) {
        if (ConstAllocator.Insert(&FieldAnnotation, Base + Offset,
                                  Base + Offset + EltSize - 1)) {
          ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                 {CbName, std::to_string(Base + Offset)});
        }
      }
    } else if (isa<ArrayType>(EltTy)) {
      if (((Offset + 15) & ~(0xf)) != Offset) {
        ValCtx.EmitFormatError(ValidationRule::SmCBufferArrayOffsetAlignment,
                               {CbName, std::to_string(Offset)});
        continue;
      }
      unsigned ArrayCount = 1;
      while (isa<ArrayType>(EltTy)) {
        ArrayCount *= EltTy->getArrayNumElements();
        EltTy = EltTy->getArrayElementType();
      }

      DxilStructAnnotation *EltAnnotation = nullptr;
      if (StructType *EltST = dyn_cast<StructType>(EltTy))
        EltAnnotation = TypeSys.GetStructAnnotation(EltST);

      unsigned AlignedEltSize = ((EltSize + 15) & ~(0xf));
      unsigned ArraySize = ((ArrayCount - 1) * AlignedEltSize) + EltSize;
      IsOutOfBound = (Offset + ArraySize) > CbSize;

      if (!IsOutOfBound) {
        // If we didn't care about gaps where elements could be placed with user
        // offsets, we could: recurse once if EltAnnotation, then allocate the
        // rest if ArrayCount > 1

        unsigned ArrayBase = Base + Offset;
        if (!EltAnnotation) {
          if (EltSize > 0 &&
              nullptr != ConstAllocator.Insert(&FieldAnnotation, ArrayBase,
                                               ArrayBase + ArraySize - 1)) {
            ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                   {CbName, std::to_string(ArrayBase)});
          }
        } else {
          for (unsigned Idx = 0; Idx < ArrayCount; Idx++) {
            CollectCBufferRanges(EltAnnotation, ConstAllocator, ArrayBase,
                                 TypeSys, CbName, ValCtx);
            ArrayBase += AlignedEltSize;
          }
        }
      }
    } else {
      StructType *EltST = cast<StructType>(EltTy);
      unsigned StructBase = Base + Offset;
      IsOutOfBound = (Offset + EltSize) > CbSize;
      if (!IsOutOfBound) {
        if (DxilStructAnnotation *EltAnnotation =
                TypeSys.GetStructAnnotation(EltST)) {
          CollectCBufferRanges(EltAnnotation, ConstAllocator, StructBase,
                               TypeSys, CbName, ValCtx);
        } else {
          if (EltSize > 0 &&
              nullptr != ConstAllocator.Insert(&FieldAnnotation, StructBase,
                                               StructBase + EltSize - 1)) {
            ValCtx.EmitFormatError(ValidationRule::SmCBufferOffsetOverlap,
                                   {CbName, std::to_string(StructBase)});
          }
        }
      }
    }

    if (IsOutOfBound) {
      ValCtx.EmitFormatError(ValidationRule::SmCBufferElementOverflow,
                             {CbName, std::to_string(Base + Offset)});
    }
  }
}

static void ValidateCBuffer(DxilCBuffer &Cb, ValidationContext &ValCtx) {
  Type *Ty = Cb.GetHLSLType()->getPointerElementType();
  if (Cb.GetRangeSize() != 1 || Ty->isArrayTy()) {
    Ty = Ty->getArrayElementType();
  }
  if (!isa<StructType>(Ty)) {
    ValCtx.EmitResourceError(&Cb,
                             ValidationRule::SmCBufferTemplateTypeMustBeStruct);
    return;
  }
  if (Cb.GetSize() > (DXIL::kMaxCBufferSize << 4)) {
    ValCtx.EmitResourceFormatError(&Cb, ValidationRule::SmCBufferSize,
                                   {std::to_string(Cb.GetSize())});
    return;
  }
  StructType *ST = cast<StructType>(Ty);
  DxilTypeSystem &TypeSys = ValCtx.DxilMod.GetTypeSystem();
  DxilStructAnnotation *Annotation = TypeSys.GetStructAnnotation(ST);
  if (!Annotation)
    return;

  // Collect constant ranges.
  std::vector<std::pair<unsigned, unsigned>> ConstRanges;
  SpanAllocator<unsigned, DxilFieldAnnotation> ConstAllocator(
      0,
      // 4096 * 16 bytes.
      DXIL::kMaxCBufferSize << 4);
  CollectCBufferRanges(Annotation, ConstAllocator, 0, TypeSys,
                       ValCtx.GetResourceName(&Cb), ValCtx);
}

static void ValidateResources(ValidationContext &ValCtx) {
  const vector<unique_ptr<DxilResource>> &Uavs = ValCtx.DxilMod.GetUAVs();
  SpacesAllocator<unsigned, DxilResourceBase> UavAllocator;

  for (auto &Uav : Uavs) {
    if (Uav->IsROV()) {
      if (!ValCtx.DxilMod.GetShaderModel()->IsPS() && !ValCtx.isLibProfile) {
        ValCtx.EmitResourceError(Uav.get(), ValidationRule::SmROVOnlyInPS);
      }
    }
    switch (Uav->GetKind()) {
    case DXIL::ResourceKind::TextureCube:
    case DXIL::ResourceKind::TextureCubeArray:
      ValCtx.EmitResourceError(Uav.get(),
                               ValidationRule::SmInvalidTextureKindOnUAV);
      break;
    default:
      break;
    }

    if (Uav->HasCounter() && !Uav->IsStructuredBuffer()) {
      ValCtx.EmitResourceError(Uav.get(),
                               ValidationRule::SmCounterOnlyOnStructBuf);
    }
    const bool UavIsCoherent =
        Uav->IsGloballyCoherent() || Uav->IsReorderCoherent();
    if (Uav->HasCounter() && UavIsCoherent) {
      StringRef Prefix = Uav->IsGloballyCoherent() ? "globally" : "reorder";
      ValCtx.EmitResourceFormatError(
          Uav.get(), ValidationRule::MetaCoherenceNotOnAppendConsume, {Prefix});
    }

    ValidateResource(*Uav, ValCtx);
    ValidateResourceOverlap(*Uav, UavAllocator, ValCtx);
  }

  SpacesAllocator<unsigned, DxilResourceBase> SrvAllocator;
  const vector<unique_ptr<DxilResource>> &Srvs = ValCtx.DxilMod.GetSRVs();
  for (auto &srv : Srvs) {
    ValidateResource(*srv, ValCtx);
    ValidateResourceOverlap(*srv, SrvAllocator, ValCtx);
  }

  hlsl::DxilResourceBase *NonDenseRes;
  if (!AreDxilResourcesDense(&ValCtx.M, &NonDenseRes)) {
    ValCtx.EmitResourceError(NonDenseRes, ValidationRule::MetaDenseResIDs);
  }

  SpacesAllocator<unsigned, DxilResourceBase> SamplerAllocator;
  for (auto &sampler : ValCtx.DxilMod.GetSamplers()) {
    if (sampler->GetSamplerKind() == DXIL::SamplerKind::Invalid) {
      ValCtx.EmitResourceError(sampler.get(),
                               ValidationRule::MetaValidSamplerMode);
    }
    ValidateResourceOverlap(*sampler, SamplerAllocator, ValCtx);
  }

  SpacesAllocator<unsigned, DxilResourceBase> CbufferAllocator;
  for (auto &cbuffer : ValCtx.DxilMod.GetCBuffers()) {
    ValidateCBuffer(*cbuffer, ValCtx);
    ValidateResourceOverlap(*cbuffer, CbufferAllocator, ValCtx);
  }
}

static void ValidateShaderFlags(ValidationContext &ValCtx) {
  ShaderFlags CalcFlags;
  ValCtx.DxilMod.CollectShaderFlagsForModule(CalcFlags);

  // Special case for validator version prior to 1.8.
  // If DXR 1.1 flag is set, but our computed flags do not have this set, then
  // this is due to prior versions setting the flag based on DXR 1.1 subobjects,
  // which are gone by this point.  Set the flag and the rest should match.
  unsigned ValMajor, ValMinor;
  ValCtx.DxilMod.GetValidatorVersion(ValMajor, ValMinor);
  if (DXIL::CompareVersions(ValMajor, ValMinor, 1, 5) >= 0 &&
      DXIL::CompareVersions(ValMajor, ValMinor, 1, 8) < 0 &&
      ValCtx.DxilMod.m_ShaderFlags.GetRaytracingTier1_1() &&
      !CalcFlags.GetRaytracingTier1_1()) {
    CalcFlags.SetRaytracingTier1_1(true);
  }

  const uint64_t Mask = ShaderFlags::GetShaderFlagsRawForCollection();
  uint64_t DeclaredFlagsRaw = ValCtx.DxilMod.m_ShaderFlags.GetShaderFlagsRaw();
  uint64_t CalcFlagsRaw = CalcFlags.GetShaderFlagsRaw();

  DeclaredFlagsRaw &= Mask;
  CalcFlagsRaw &= Mask;

  if (DeclaredFlagsRaw == CalcFlagsRaw) {
    return;
  }
  ValCtx.EmitError(ValidationRule::MetaFlagsUsage);

  dxilutil::EmitNoteOnContext(ValCtx.M.getContext(),
                              Twine("Flags declared=") +
                                  Twine(DeclaredFlagsRaw) + Twine(", actual=") +
                                  Twine(CalcFlagsRaw));
}

static void ValidateSignatureElement(DxilSignatureElement &SE,
                                     ValidationContext &ValCtx) {
  DXIL::SemanticKind SemanticKind = SE.GetSemantic()->GetKind();
  CompType::Kind CompKind = SE.GetCompType().GetKind();
  DXIL::InterpolationMode Mode = SE.GetInterpolationMode()->GetKind();

  StringRef Name = SE.GetName();
  if (Name.size() < 1 || Name.size() > 64) {
    ValCtx.EmitSignatureError(&SE, ValidationRule::MetaSemanticLen);
  }

  if (SemanticKind > DXIL::SemanticKind::Arbitrary &&
      SemanticKind < DXIL::SemanticKind::Invalid) {
    if (SemanticKind != Semantic::GetByName(SE.GetName())->GetKind()) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemaKindMatchesName,
                             {SE.GetName(), SE.GetSemantic()->GetName()});
    }
  }

  unsigned CompWidth = 0;
  bool CompFloat = false;
  bool CompInt = false;
  bool CompBool = false;

  switch (CompKind) {
  case CompType::Kind::U64:
    CompWidth = 64;
    CompInt = true;
    break;
  case CompType::Kind::I64:
    CompWidth = 64;
    CompInt = true;
    break;
  // These should be translated for signatures:
  // case CompType::Kind::PackedS8x32:
  // case CompType::Kind::PackedU8x32:
  case CompType::Kind::U32:
    CompWidth = 32;
    CompInt = true;
    break;
  case CompType::Kind::I32:
    CompWidth = 32;
    CompInt = true;
    break;
  case CompType::Kind::U16:
    CompWidth = 16;
    CompInt = true;
    break;
  case CompType::Kind::I16:
    CompWidth = 16;
    CompInt = true;
    break;
  case CompType::Kind::I1:
    CompWidth = 1;
    CompBool = true;
    break;
  case CompType::Kind::F64:
    CompWidth = 64;
    CompFloat = true;
    break;
  case CompType::Kind::F32:
    CompWidth = 32;
    CompFloat = true;
    break;
  case CompType::Kind::F16:
    CompWidth = 16;
    CompFloat = true;
    break;
  case CompType::Kind::SNormF64:
    CompWidth = 64;
    CompFloat = true;
    break;
  case CompType::Kind::SNormF32:
    CompWidth = 32;
    CompFloat = true;
    break;
  case CompType::Kind::SNormF16:
    CompWidth = 16;
    CompFloat = true;
    break;
  case CompType::Kind::UNormF64:
    CompWidth = 64;
    CompFloat = true;
    break;
  case CompType::Kind::UNormF32:
    CompWidth = 32;
    CompFloat = true;
    break;
  case CompType::Kind::UNormF16:
    CompWidth = 16;
    CompFloat = true;
    break;
  case CompType::Kind::Invalid:
  default:
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureCompType,
                           {SE.GetName()});
    break;
  }

  if (CompInt || CompBool) {
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
  bool AllowedInSig = true;
  bool ShouldBeAllocated = true;
  switch (SE.GetInterpretation()) {
  case DXIL::SemanticInterpretationKind::NA:
  case DXIL::SemanticInterpretationKind::NotInSig:
  case DXIL::SemanticInterpretationKind::Invalid:
    AllowedInSig = false;
    LLVM_FALLTHROUGH;
  case DXIL::SemanticInterpretationKind::NotPacked:
  case DXIL::SemanticInterpretationKind::Shadow:
    ShouldBeAllocated = false;
    break;
  default:
    break;
  }

  const char *InputOutput = nullptr;
  if (SE.IsInput())
    InputOutput = "Input";
  else if (SE.IsOutput())
    InputOutput = "Output";
  else
    InputOutput = "PatchConstant";

  if (!AllowedInSig) {
    ValCtx.EmitFormatError(ValidationRule::SmSemantic,
                           {SE.GetName(),
                            ValCtx.DxilMod.GetShaderModel()->GetKindName(),
                            InputOutput});
  } else if (ShouldBeAllocated && !SE.IsAllocated()) {
    ValCtx.EmitFormatError(ValidationRule::MetaSemanticShouldBeAllocated,
                           {InputOutput, SE.GetName()});
  } else if (!ShouldBeAllocated && SE.IsAllocated()) {
    ValCtx.EmitFormatError(ValidationRule::MetaSemanticShouldNotBeAllocated,
                           {InputOutput, SE.GetName()});
  }

  bool IsClipCull = false;
  bool IsTessfactor = false;
  bool IsBarycentric = false;

  switch (SemanticKind) {
  case DXIL::SemanticKind::Depth:
  case DXIL::SemanticKind::DepthGreaterEqual:
  case DXIL::SemanticKind::DepthLessEqual:
    if (!CompFloat || CompWidth > 32 || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    break;
  case DXIL::SemanticKind::Coverage:
    DXASSERT(!SE.IsInput() || !AllowedInSig,
             "else internal inconsistency between semantic interpretation "
             "table and validation code");
    LLVM_FALLTHROUGH;
  case DXIL::SemanticKind::InnerCoverage:
  case DXIL::SemanticKind::OutputControlPointID:
    if (CompKind != CompType::Kind::U32 || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "uint"});
    }
    break;
  case DXIL::SemanticKind::Position:
    if (!CompFloat || CompWidth > 32 || SE.GetCols() != 4) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float4"});
    }
    break;
  case DXIL::SemanticKind::Target:
    if (CompWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float/int/uint"});
    }
    break;
  case DXIL::SemanticKind::ClipDistance:
  case DXIL::SemanticKind::CullDistance:
    IsClipCull = true;
    if (!CompFloat || CompWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    // NOTE: clip cull distance size is checked at ValidateSignature.
    break;
  case DXIL::SemanticKind::IsFrontFace: {
    if (!(CompInt && CompWidth == 32) || SE.GetCols() != 1) {
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
    if ((CompKind != CompType::Kind::U32 && CompKind != CompType::Kind::U16) ||
        SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "uint"});
    }
    break;
  case DXIL::SemanticKind::CullPrimitive: {
    if (!(CompBool && CompWidth == 1) || SE.GetCols() != 1) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "bool"});
    }
  } break;
  case DXIL::SemanticKind::TessFactor:
  case DXIL::SemanticKind::InsideTessFactor:
    // NOTE: the size check is at CheckPatchConstantSemantic.
    IsTessfactor = true;
    if (!CompFloat || CompWidth > 32) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticCompType,
                             {SE.GetSemantic()->GetName(), "float"});
    }
    break;
  case DXIL::SemanticKind::Arbitrary:
    break;
  case DXIL::SemanticKind::DomainLocation:
  case DXIL::SemanticKind::Invalid:
    DXASSERT(!AllowedInSig, "else internal inconsistency between semantic "
                            "interpretation table and validation code");
    break;
  case DXIL::SemanticKind::Barycentrics:
    IsBarycentric = true;
    if (!CompFloat || CompWidth > 32) {
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

  if (SemanticKind == DXIL::SemanticKind::Target) {
    // Verify packed Row == semantic index
    unsigned Row = SE.GetStartRow();
    for (unsigned i : SE.GetSemanticIndexVec()) {
      if (Row != i) {
        ValCtx.EmitSignatureError(&SE,
                                  ValidationRule::SmPSTargetIndexMatchesRow);
      }
      ++Row;
    }
    // Verify packed Col is 0
    if (SE.GetStartCol() != 0) {
      ValCtx.EmitSignatureError(&SE, ValidationRule::SmPSTargetCol0);
    }
    // Verify max Row used < 8
    if (SE.GetStartRow() + SE.GetRows() > 8) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                             {"SV_Target", "7"});
    }
  } else if (AllowedInSig && SemanticKind != DXIL::SemanticKind::Arbitrary) {
    if (IsBarycentric) {
      if (SE.GetSemanticStartIndex() > 1) {
        ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                               {SE.GetSemantic()->GetName(), "1"});
      }
    } else if (!IsClipCull && SE.GetSemanticStartIndex() > 0) {
      ValCtx.EmitFormatError(ValidationRule::MetaSemanticIndexMax,
                             {SE.GetSemantic()->GetName(), "0"});
    }
    // Maximum rows is 1 for system values other than Target
    // with the exception of tessfactors, which are validated in
    // CheckPatchConstantSemantic and ClipDistance/CullDistance, which have
    // other custom constraints.
    if (!IsTessfactor && !IsClipCull && SE.GetRows() > 1) {
      ValCtx.EmitSignatureError(&SE, ValidationRule::MetaSystemValueRows);
    }
  }

  if (SE.GetCols() + (SE.IsAllocated() ? SE.GetStartCol() : 0) > 4) {
    unsigned Size = (SE.GetRows() - 1) * 4 + SE.GetCols();
    ValCtx.EmitFormatError(ValidationRule::MetaSignatureOutOfRange,
                           {SE.GetName(), std::to_string(SE.GetStartRow()),
                            std::to_string(SE.GetStartCol()),
                            std::to_string(Size)});
  }

  if (!SE.GetInterpolationMode()->IsValid()) {
    ValCtx.EmitSignatureError(&SE, ValidationRule::MetaInterpModeValid);
  }
}

static void ValidateSignatureOverlap(DxilSignatureElement &E,
                                     unsigned MaxScalars,
                                     DxilSignatureAllocator &Allocator,
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

  DxilPackElement PE(&E, Allocator.UseMinPrecision());
  DxilSignatureAllocator::ConflictType Conflict =
      Allocator.DetectRowConflict(&PE, E.GetStartRow());
  if (Conflict == DxilSignatureAllocator::kNoConflict ||
      Conflict == DxilSignatureAllocator::kInsufficientFreeComponents)
    Conflict =
        Allocator.DetectColConflict(&PE, E.GetStartRow(), E.GetStartCol());
  switch (Conflict) {
  case DxilSignatureAllocator::kNoConflict:
    Allocator.PlaceElement(&PE, E.GetStartRow(), E.GetStartCol());
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
                              EntryStatus &Status, unsigned MaxScalars) {
  DxilSignatureAllocator Allocator[DXIL::kNumOutputStreams] = {
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()},
      {32, ValCtx.DxilMod.GetUseMinPrecision()}};
  unordered_set<unsigned> SemanticUsageSet[DXIL::kNumOutputStreams];
  StringMap<unordered_set<unsigned>> SemanticIndexMap[DXIL::kNumOutputStreams];
  unordered_set<unsigned> ClipcullRowSet[DXIL::kNumOutputStreams];
  unsigned ClipcullComponents[DXIL::kNumOutputStreams] = {0, 0, 0, 0};

  bool IsOutput = S.IsOutput();
  unsigned TargetMask = 0;
  DXIL::SemanticKind DepthKind = DXIL::SemanticKind::Invalid;

  const InterpolationMode *PrevBaryInterpMode = nullptr;
  unsigned NumBarycentrics = 0;

  for (auto &E : S.GetElements()) {
    DXIL::SemanticKind SemanticKind = E->GetSemantic()->GetKind();
    ValidateSignatureElement(*E, ValCtx);
    // Avoid OOB indexing on StreamId.
    unsigned StreamId = E->GetOutputStream();
    if (StreamId >= DXIL::kNumOutputStreams || !IsOutput ||
        !ValCtx.DxilMod.GetShaderModel()->IsGS()) {
      StreamId = 0;
    }

    // Semantic index overlap check, keyed by name.
    std::string NameUpper(E->GetName());
    std::transform(NameUpper.begin(), NameUpper.end(), NameUpper.begin(),
                   ::toupper);
    unordered_set<unsigned> &SemIdxSet = SemanticIndexMap[StreamId][NameUpper];
    for (unsigned SemIdx : E->GetSemanticIndexVec()) {
      if (SemIdxSet.count(SemIdx) > 0) {
        ValCtx.EmitFormatError(ValidationRule::MetaNoSemanticOverlap,
                               {E->GetName(), std::to_string(SemIdx)});
        return;
      } else
        SemIdxSet.insert(SemIdx);
    }

    // SV_Target has special rules
    if (SemanticKind == DXIL::SemanticKind::Target) {
      // Validate target overlap
      if (E->GetStartRow() + E->GetRows() <= 8) {
        unsigned Mask = ((1 << E->GetRows()) - 1) << E->GetStartRow();
        if (TargetMask & Mask) {
          ValCtx.EmitFormatError(
              ValidationRule::MetaNoSemanticOverlap,
              {"SV_Target", std::to_string(E->GetStartRow())});
        }
        TargetMask = TargetMask | Mask;
      }
      if (E->GetRows() > 1) {
        ValCtx.EmitSignatureError(E.get(), ValidationRule::SmNoPSOutputIdx);
      }
      continue;
    }

    if (E->GetSemantic()->IsInvalid())
      continue;

    // validate system value semantic rules
    switch (SemanticKind) {
    case DXIL::SemanticKind::Arbitrary:
      break;
    case DXIL::SemanticKind::ClipDistance:
    case DXIL::SemanticKind::CullDistance:
      // Validate max 8 components across 2 rows (registers)
      for (unsigned RowIdx = 0; RowIdx < E->GetRows(); RowIdx++)
        ClipcullRowSet[StreamId].insert(E->GetStartRow() + RowIdx);
      if (ClipcullRowSet[StreamId].size() > 2) {
        ValCtx.EmitSignatureError(E.get(), ValidationRule::MetaClipCullMaxRows);
      }
      ClipcullComponents[StreamId] += E->GetCols();
      if (ClipcullComponents[StreamId] > 8) {
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
      DepthKind = SemanticKind;
      break;
    case DXIL::SemanticKind::Barycentrics: {
      // There can only be up to two SV_Barycentrics
      // with differeent perspective interpolation modes.
      if (NumBarycentrics++ > 1) {
        ValCtx.EmitSignatureError(
            E.get(), ValidationRule::MetaBarycentricsTwoPerspectives);
        break;
      }
      const InterpolationMode *Mode = E->GetInterpolationMode();
      if (PrevBaryInterpMode) {
        if ((Mode->IsAnyNoPerspective() &&
             PrevBaryInterpMode->IsAnyNoPerspective()) ||
            (!Mode->IsAnyNoPerspective() &&
             !PrevBaryInterpMode->IsAnyNoPerspective())) {
          ValCtx.EmitSignatureError(
              E.get(), ValidationRule::MetaBarycentricsTwoPerspectives);
        }
      }
      PrevBaryInterpMode = Mode;
      break;
    }
    default:
      if (SemanticUsageSet[StreamId].count(
              static_cast<unsigned>(SemanticKind)) > 0) {
        ValCtx.EmitFormatError(ValidationRule::MetaDuplicateSysValue,
                               {E->GetSemantic()->GetName()});
      }
      SemanticUsageSet[StreamId].insert(static_cast<unsigned>(SemanticKind));
      break;
    }

    // Packed element overlap check.
    ValidateSignatureOverlap(*E.get(), MaxScalars, Allocator[StreamId], ValCtx);

    if (IsOutput && SemanticKind == DXIL::SemanticKind::Position) {
      Status.hasOutputPosition[E->GetOutputStream()] = true;
    }
  }

  if (Status.hasViewID && S.IsInput() &&
      ValCtx.DxilMod.GetShaderModel()->GetKind() == DXIL::ShaderKind::Pixel) {
    // Ensure sufficient space for ViewId:
    DxilSignatureAllocator::DummyElement ViewId;
    ViewId.rows = 1;
    ViewId.cols = 1;
    ViewId.kind = DXIL::SemanticKind::Arbitrary;
    ViewId.interpolation = DXIL::InterpolationMode::Constant;
    ViewId.interpretation = DXIL::SemanticInterpretationKind::SGV;
    Allocator[0].PackNext(&ViewId, 0, 32);
    if (!ViewId.IsAllocated()) {
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
                                    const DxilEntryProps &EntryProps,
                                    EntryStatus &Status, Function &F) {
  const DxilFunctionProps &Props = EntryProps.props;
  const DxilEntrySignature &S = EntryProps.sig;

  if (Props.IsRay()) {
    // No signatures allowed
    if (!S.InputSignature.GetElements().empty() ||
        !S.OutputSignature.GetElements().empty() ||
        !S.PatchConstOrPrimSignature.GetElements().empty()) {
      ValCtx.EmitFnFormatError(&F, ValidationRule::SmRayShaderSignatures,
                               {F.getName()});
    }

    // Validate payload/attribute/params sizes
    unsigned PayloadSize = 0;
    unsigned AttrSize = 0;
    auto ItPayload = F.arg_begin();
    auto ItAttr = ItPayload;
    if (ItAttr != F.arg_end())
      ItAttr++;
    DataLayout DL(F.getParent());
    switch (Props.shaderKind) {
    case DXIL::ShaderKind::AnyHit:
    case DXIL::ShaderKind::ClosestHit:
      if (ItAttr != F.arg_end()) {
        Type *Ty = ItAttr->getType();
        if (Ty->isPointerTy())
          Ty = Ty->getPointerElementType();
        AttrSize =
            (unsigned)std::min(DL.getTypeAllocSize(Ty), (uint64_t)UINT_MAX);
      }
      LLVM_FALLTHROUGH;
    case DXIL::ShaderKind::Miss:
    case DXIL::ShaderKind::Callable:
      if (ItPayload != F.arg_end()) {
        Type *Ty = ItPayload->getType();
        if (Ty->isPointerTy())
          Ty = Ty->getPointerElementType();
        PayloadSize =
            (unsigned)std::min(DL.getTypeAllocSize(Ty), (uint64_t)UINT_MAX);
      }
      break;
    }
    if (Props.ShaderProps.Ray.payloadSizeInBytes < PayloadSize) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmRayShaderPayloadSize,
          {F.getName(), Props.IsCallable() ? "params" : "payload"});
    }
    if (Props.ShaderProps.Ray.attributeSizeInBytes < AttrSize) {
      ValCtx.EmitFnFormatError(&F, ValidationRule::SmRayShaderPayloadSize,
                               {F.getName(), "attribute"});
    }
    return;
  }

  bool IsPs = Props.IsPS();
  bool IsVs = Props.IsVS();
  bool IsGs = Props.IsGS();
  bool IsCs = Props.IsCS();
  bool IsMs = Props.IsMS();

  if (IsPs) {
    // PS output no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.OutputSignature);
  } else if (IsVs) {
    // VS input no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.InputSignature);
  }

  if (IsMs) {
    // primitive output constant interp mode.
    ValidateConstantInterpModeSignature(ValCtx, S.PatchConstOrPrimSignature);
  } else {
    // patch constant no interp mode.
    ValidateNoInterpModeSignature(ValCtx, S.PatchConstOrPrimSignature);
  }

  unsigned MaxInputScalars = DXIL::kMaxInputTotalScalars;
  unsigned MaxOutputScalars = 0;
  unsigned MaxPatchConstantScalars = 0;

  switch (Props.shaderKind) {
  case DXIL::ShaderKind::Compute:
    break;
  case DXIL::ShaderKind::Vertex:
  case DXIL::ShaderKind::Geometry:
  case DXIL::ShaderKind::Pixel:
    MaxOutputScalars = DXIL::kMaxOutputTotalScalars;
    break;
  case DXIL::ShaderKind::Hull:
  case DXIL::ShaderKind::Domain:
    MaxOutputScalars = DXIL::kMaxOutputTotalScalars;
    MaxPatchConstantScalars = DXIL::kMaxHSOutputPatchConstantTotalScalars;
    break;
  case DXIL::ShaderKind::Mesh:
    MaxOutputScalars = DXIL::kMaxOutputTotalScalars;
    MaxPatchConstantScalars = DXIL::kMaxOutputTotalScalars;
    break;
  case DXIL::ShaderKind::Amplification:
  default:
    break;
  }

  ValidateSignature(ValCtx, S.InputSignature, Status, MaxInputScalars);
  ValidateSignature(ValCtx, S.OutputSignature, Status, MaxOutputScalars);
  ValidateSignature(ValCtx, S.PatchConstOrPrimSignature, Status,
                    MaxPatchConstantScalars);

  if (IsPs) {
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
    const DxilSignature &OutputSig = S.OutputSignature;
    for (auto &SE : OutputSig.GetElements()) {
      Semantic::Kind SemanticKind = SE->GetSemantic()->GetKind();
      switch (SemanticKind) {
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

  if (IsGs) {
    unsigned MaxVertexCount = Props.ShaderProps.GS.maxVertexCount;
    unsigned OutputScalarCount = 0;
    const DxilSignature &OutSig = S.OutputSignature;
    for (auto &SE : OutSig.GetElements()) {
      OutputScalarCount += SE->GetRows() * SE->GetCols();
    }
    unsigned TotalOutputScalars = MaxVertexCount * OutputScalarCount;
    if (TotalOutputScalars > DXIL::kMaxGSOutputTotalScalars) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmGSTotalOutputVertexDataRange,
          {std::to_string(MaxVertexCount), std::to_string(OutputScalarCount),
           std::to_string(TotalOutputScalars),
           std::to_string(DXIL::kMaxGSOutputTotalScalars)});
    }
  }

  if (IsCs) {
    if (!S.InputSignature.GetElements().empty() ||
        !S.OutputSignature.GetElements().empty() ||
        !S.PatchConstOrPrimSignature.GetElements().empty()) {
      ValCtx.EmitFnError(&F, ValidationRule::SmCSNoSignatures);
    }
  }

  if (IsMs) {
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
    unsigned MaxAlign32VertexCount =
        ALIGN32(Props.ShaderProps.MS.maxVertexCount);
    unsigned MaxAlign32PrimitiveCount =
        ALIGN32(Props.ShaderProps.MS.maxPrimitiveCount);
    unsigned TotalOutputScalars = 0;
    for (auto &SE : S.OutputSignature.GetElements()) {
      TotalOutputScalars +=
          SE->GetRows() * SE->GetCols() * MaxAlign32VertexCount;
    }
    for (auto &SE : S.PatchConstOrPrimSignature.GetElements()) {
      TotalOutputScalars +=
          SE->GetRows() * SE->GetCols() * MaxAlign32PrimitiveCount;
    }

    if (TotalOutputScalars * kScalarSizeForMSAttributes >
        DXIL::kMaxMSOutputTotalBytes) {
      ValCtx.EmitFnFormatError(
          &F, ValidationRule::SmMeshShaderOutputSize,
          {F.getName(), std::to_string(DXIL::kMaxMSOutputTotalBytes)});
    }

    unsigned TotalInputOutputBytes =
        TotalOutputScalars * kScalarSizeForMSAttributes +
        Props.ShaderProps.MS.payloadSizeInBytes;
    if (TotalInputOutputBytes > DXIL::kMaxMSInputOutputTotalBytes) {
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
        DxilEntryProps &EntryProps = DM.GetDxilEntryProps(&F);
        EntryStatus &Status = ValCtx.GetEntryStatus(&F);
        ValidateEntrySignatures(ValCtx, EntryProps, Status, F);
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
    DxilEntryProps &EntryProps = DM.GetDxilEntryProps(Entry);
    ValidateEntrySignatures(ValCtx, EntryProps, Status, *Entry);
  }
}

// CompatibilityChecker is used to identify incompatibilities in an entry
// function and any functions called by that entry function.
struct CompatibilityChecker {
  ValidationContext &ValCtx;
  Function *EntryFn;
  const DxilFunctionProps &Props;
  DXIL::ShaderKind ShaderKind;

  // These masks identify the potential conflict flags based on the entry
  // function's shader kind and properties when either UsesDerivatives or
  // RequiresGroup flags are set in ShaderCompatInfo.
  uint32_t MaskForDeriv = 0;
  uint32_t MaskForGroup = 0;

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
        Props(ValCtx.DxilMod.GetDxilEntryProps(EntryFn).props),
        ShaderKind(Props.shaderKind) {

    // Precompute potential incompatibilities based on shader stage, shader kind
    // and entry attributes. These will turn into full conflicts if the entry
    // point's shader flags indicate that they use relevant features.
    if (!ValCtx.DxilMod.GetShaderModel()->IsSM66Plus() &&
        (ShaderKind == DXIL::ShaderKind::Mesh ||
         ShaderKind == DXIL::ShaderKind::Amplification ||
         ShaderKind == DXIL::ShaderKind::Compute)) {
      MaskForDeriv |=
          static_cast<uint32_t>(ConflictFlags::DerivInComputeShaderModel);
    } else if (ShaderKind == DXIL::ShaderKind::Node) {
      // Only broadcasting launch supports derivatives.
      if (Props.Node.LaunchType != DXIL::NodeLaunchType::Broadcasting)
        MaskForDeriv |= static_cast<uint32_t>(ConflictFlags::DerivLaunch);
      // Thread launch node has no group.
      if (Props.Node.LaunchType == DXIL::NodeLaunchType::Thread)
        MaskForGroup |= static_cast<uint32_t>(ConflictFlags::RequiresGroup);
    }

    if (ShaderKind == DXIL::ShaderKind::Mesh ||
        ShaderKind == DXIL::ShaderKind::Amplification ||
        ShaderKind == DXIL::ShaderKind::Compute ||
        ShaderKind == DXIL::ShaderKind::Node) {
      // All compute-like stages
      // Thread dimensions must be either 1D and X is multiple of 4, or 2D
      // and X and Y must be multiples of 2.
      if (Props.numThreads[1] == 1 && Props.numThreads[2] == 1) {
        if ((Props.numThreads[0] & 0x3) != 0)
          MaskForDeriv |=
              static_cast<uint32_t>(ConflictFlags::DerivThreadGroupDim);
      } else if ((Props.numThreads[0] & 0x1) || (Props.numThreads[1] & 0x1))
        MaskForDeriv |=
            static_cast<uint32_t>(ConflictFlags::DerivThreadGroupDim);
    } else {
      // other stages have no group
      MaskForGroup |= static_cast<uint32_t>(ConflictFlags::RequiresGroup);
    }
  }

  uint32_t
  IdentifyConflict(const DxilModule::ShaderCompatInfo &CompatInfo) const {
    uint32_t ConflictMask = 0;

    // Compatibility check said this shader kind is not compatible.
    if (0 == ((1 << (uint32_t)ShaderKind) & CompatInfo.mask))
      ConflictMask |= (uint32_t)ConflictFlags::Stage;

    // Compatibility check said this shader model is not compatible.
    if (DXIL::CompareVersions(ValCtx.DxilMod.GetShaderModel()->GetMajor(),
                              ValCtx.DxilMod.GetShaderModel()->GetMinor(),
                              CompatInfo.minMajor, CompatInfo.minMinor) < 0)
      ConflictMask |= (uint32_t)ConflictFlags::ShaderModel;

    if (CompatInfo.shaderFlags.GetUsesDerivatives())
      ConflictMask |= MaskForDeriv;

    if (CompatInfo.shaderFlags.GetRequiresGroup())
      ConflictMask |= MaskForGroup;

    return ConflictMask;
  }

  void Diagnose(Function *F, uint32_t ConflictMask, ConflictKind Conflict,
                ValidationRule Rule, ArrayRef<StringRef> Args = {}) {
    if (ConflictMask & (1 << (unsigned)Conflict))
      ValCtx.EmitFnFormatError(F, Rule, Args);
  }

  void DiagnoseConflicts(Function *F, uint32_t ConflictMask) {
    // Emit a diagnostic indicating that either the entry function or a function
    // called by the entry function contains a disallowed operation.
    if (F == EntryFn)
      ValCtx.EmitFnError(EntryFn, ValidationRule::SmIncompatibleOperation);
    else
      ValCtx.EmitFnError(EntryFn, ValidationRule::SmIncompatibleCallInEntry);

    // Emit diagnostics for each conflict found in this function.
    Diagnose(F, ConflictMask, ConflictKind::Stage,
             ValidationRule::SmIncompatibleStage,
             {ShaderModel::GetKindName(Props.shaderKind)});
    Diagnose(F, ConflictMask, ConflictKind::ShaderModel,
             ValidationRule::SmIncompatibleShaderModel);
    Diagnose(F, ConflictMask, ConflictKind::DerivLaunch,
             ValidationRule::SmIncompatibleDerivLaunch,
             {GetLaunchTypeStr(Props.Node.LaunchType)});
    Diagnose(F, ConflictMask, ConflictKind::DerivThreadGroupDim,
             ValidationRule::SmIncompatibleThreadGroupDim,
             {std::to_string(Props.numThreads[0]),
              std::to_string(Props.numThreads[1]),
              std::to_string(Props.numThreads[2])});
    Diagnose(F, ConflictMask, ConflictKind::DerivInComputeShaderModel,
             ValidationRule::SmIncompatibleDerivInComputeShaderModel);
    Diagnose(F, ConflictMask, ConflictKind::RequiresGroup,
             ValidationRule::SmIncompatibleRequiresGroup);
  }

  // Visit function and all functions called by it.
  // Emit diagnostics for incompatibilities found in a function when no
  // functions called by that function introduced the conflict.
  // In those cases, the called functions themselves will emit the diagnostic.
  // Return conflict mask for this function.
  uint32_t Visit(Function *F, uint32_t &RemainingMask,
                 llvm::SmallPtrSet<Function *, 8> &Visited, CallGraph &CG) {
    // Recursive check looks for where a conflict is found and not present
    // in functions called by the current function.
    // - When a source is found, emit diagnostics and clear the conflict
    // flags introduced by this function from the working mask so we don't
    // report this conflict again.
    // - When the RemainingMask is 0, we are done.

    if (RemainingMask == 0)
      return 0; // Nothing left to search for.
    if (!Visited.insert(F).second)
      return 0; // Already visited.

    const DxilModule::ShaderCompatInfo *CompatInfo =
        ValCtx.DxilMod.GetCompatInfoForFunction(F);
    DXASSERT(CompatInfo, "otherwise, compat info not computed in module");
    if (!CompatInfo)
      return 0;
    uint32_t MaskForThisFunction = IdentifyConflict(*CompatInfo);

    uint32_t MaskForCalls = 0;
    if (CallGraphNode *CGNode = CG[F]) {
      for (auto &Call : *CGNode) {
        Function *called = Call.second->getFunction();
        if (called->isDeclaration())
          continue;
        MaskForCalls |= Visit(called, RemainingMask, Visited, CG);
        if (RemainingMask == 0)
          return 0; // Nothing left to search for.
      }
    }

    // Mask of incompatibilities introduced by this function.
    uint32_t ConflictsIntroduced =
        RemainingMask & MaskForThisFunction & ~MaskForCalls;
    if (ConflictsIntroduced) {
      // This function introduces at least one conflict.
      DiagnoseConflicts(F, ConflictsIntroduced);
      // Mask off diagnosed incompatibilities.
      RemainingMask &= ~ConflictsIntroduced;
    }
    return MaskForThisFunction;
  }

  void FindIncompatibleCall(const DxilModule::ShaderCompatInfo &CompatInfo) {
    uint32_t ConflictMask = IdentifyConflict(CompatInfo);
    if (ConflictMask == 0)
      return;

    CallGraph &CG = ValCtx.GetCallGraph();
    llvm::SmallPtrSet<Function *, 8> Visited;
    Visit(EntryFn, ConflictMask, Visited, CG);
  }
};

static void ValidateEntryCompatibility(ValidationContext &ValCtx) {
  // Make sure functions called from each entry are compatible with that entry.
  DxilModule &DM = ValCtx.DxilMod;
  for (Function &F : DM.GetModule()->functions()) {
    if (DM.HasDxilEntryProps(&F)) {
      const DxilModule::ShaderCompatInfo *CompatInfo =
          DM.GetCompatInfoForFunction(&F);
      DXASSERT(CompatInfo, "otherwise, compat info not computed in module");
      if (!CompatInfo)
        continue;

      CompatibilityChecker checker(ValCtx, &F);
      checker.FindIncompatibleCall(*CompatInfo);
    }
  }
}

static void CheckPatchConstantSemantic(ValidationContext &ValCtx,
                                       const DxilEntryProps &EntryProps,
                                       EntryStatus &Status, Function *F) {
  const DxilFunctionProps &Props = EntryProps.props;
  bool IsHs = Props.IsHS();

  DXIL::TessellatorDomain Domain =
      IsHs ? Props.ShaderProps.HS.domain : Props.ShaderProps.DS.domain;

  const DxilSignature &PatchConstantSig =
      EntryProps.sig.PatchConstOrPrimSignature;

  const unsigned KQuadEdgeSize = 4;
  const unsigned KQuadInsideSize = 2;
  const unsigned KQuadDomainLocSize = 2;

  const unsigned KTriEdgeSize = 3;
  const unsigned KTriInsideSize = 1;
  const unsigned KTriDomainLocSize = 3;

  const unsigned KIsolineEdgeSize = 2;
  const unsigned KIsolineInsideSize = 0;
  const unsigned KIsolineDomainLocSize = 3;

  const char *DomainName = "";

  DXIL::SemanticKind kEdgeSemantic = DXIL::SemanticKind::TessFactor;
  unsigned EdgeSize = 0;

  DXIL::SemanticKind kInsideSemantic = DXIL::SemanticKind::InsideTessFactor;
  unsigned InsideSize = 0;

  Status.domainLocSize = 0;

  switch (Domain) {
  case DXIL::TessellatorDomain::IsoLine:
    DomainName = "IsoLine";
    EdgeSize = KIsolineEdgeSize;
    InsideSize = KIsolineInsideSize;
    Status.domainLocSize = KIsolineDomainLocSize;
    break;
  case DXIL::TessellatorDomain::Tri:
    DomainName = "Tri";
    EdgeSize = KTriEdgeSize;
    InsideSize = KTriInsideSize;
    Status.domainLocSize = KTriDomainLocSize;
    break;
  case DXIL::TessellatorDomain::Quad:
    DomainName = "Quad";
    EdgeSize = KQuadEdgeSize;
    InsideSize = KQuadInsideSize;
    Status.domainLocSize = KQuadDomainLocSize;
    break;
  default:
    // Don't bother with other tests if domain is invalid
    return;
  }

  bool FoundEdgeSemantic = false;
  bool FoundInsideSemantic = false;
  for (auto &SE : PatchConstantSig.GetElements()) {
    Semantic::Kind Kind = SE->GetSemantic()->GetKind();
    if (Kind == kEdgeSemantic) {
      FoundEdgeSemantic = true;
      if (SE->GetRows() != EdgeSize || SE->GetCols() > 1) {
        ValCtx.EmitFnFormatError(F, ValidationRule::SmTessFactorSizeMatchDomain,
                                 {std::to_string(SE->GetRows()),
                                  std::to_string(SE->GetCols()), DomainName,
                                  std::to_string(EdgeSize)});
      }
    } else if (Kind == kInsideSemantic) {
      FoundInsideSemantic = true;
      if (SE->GetRows() != InsideSize || SE->GetCols() > 1) {
        ValCtx.EmitFnFormatError(
            F, ValidationRule::SmInsideTessFactorSizeMatchDomain,
            {std::to_string(SE->GetRows()), std::to_string(SE->GetCols()),
             DomainName, std::to_string(InsideSize)});
      }
    }
  }

  if (IsHs) {
    if (!FoundEdgeSemantic) {
      ValCtx.EmitFnError(F, ValidationRule::SmTessFactorForDomain);
    }
    if (!FoundInsideSemantic && Domain != DXIL::TessellatorDomain::IsoLine) {
      ValCtx.EmitFnError(F, ValidationRule::SmTessFactorForDomain);
    }
  }
}

static void ValidatePassThruHS(ValidationContext &ValCtx,
                               const DxilEntryProps &EntryProps, Function *F) {
  // Check pass thru HS.
  if (F->isDeclaration()) {
    const auto &Props = EntryProps.props;
    if (Props.IsHS()) {
      const auto &HS = Props.ShaderProps.HS;
      if (HS.inputControlPoints < HS.outputControlPoints) {
        ValCtx.EmitFnError(
            F, ValidationRule::SmHullPassThruControlPointCountMatch);
      }

      // Check declared control point outputs storage amounts are ok to pass
      // through (less output storage than input for control points).
      const DxilSignature &OutSig = EntryProps.sig.OutputSignature;
      unsigned TotalOutputCpScalars = 0;
      for (auto &SE : OutSig.GetElements()) {
        TotalOutputCpScalars += SE->GetRows() * SE->GetCols();
      }
      if (TotalOutputCpScalars * HS.outputControlPoints >
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
                             const DxilEntryProps &EntryProps, Function *F) {
  const DxilFunctionProps &Props = EntryProps.props;
  const hlsl::DxilWaveSize &WaveSize = Props.WaveSize;

  switch (WaveSize.Validate()) {
  case hlsl::DxilWaveSize::ValidationResult::Success:
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidMin:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Min", std::to_string(WaveSize.Min),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidMax:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Max", std::to_string(WaveSize.Max),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::InvalidPreferred:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizeValue,
                             {"Preferred", std::to_string(WaveSize.Preferred),
                              std::to_string(DXIL::kMinWaveSize),
                              std::to_string(DXIL::kMaxWaveSize)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxOrPreferredWhenUndefined:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeAllZeroWhenUndefined,
        {std::to_string(WaveSize.Max), std::to_string(WaveSize.Preferred)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxEqualsMin:
    // This case is allowed because users may disable the ErrorDefault warning.
    break;
  case hlsl::DxilWaveSize::ValidationResult::PreferredWhenNoRange:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeMaxAndPreferredZeroWhenNoRange,
        {std::to_string(WaveSize.Max), std::to_string(WaveSize.Preferred)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::MaxLessThanMin:
    ValCtx.EmitFnFormatError(
        F, ValidationRule::SmWaveSizeMaxGreaterThanMin,
        {std::to_string(WaveSize.Max), std::to_string(WaveSize.Min)});
    break;
  case hlsl::DxilWaveSize::ValidationResult::PreferredOutOfRange:
    ValCtx.EmitFnFormatError(F, ValidationRule::SmWaveSizePreferredInRange,
                             {std::to_string(WaveSize.Preferred),
                              std::to_string(WaveSize.Min),
                              std::to_string(WaveSize.Max)});
    break;
  }

  // Check shader model and kind.
  if (WaveSize.IsDefined()) {
    if (!Props.IsCS() && !Props.IsNode()) {
      ValCtx.EmitFnError(F, ValidationRule::SmWaveSizeOnComputeOrNode);
    }
  }
}

static void ValidateEntryProps(ValidationContext &ValCtx,
                               const DxilEntryProps &EntryProps,
                               EntryStatus &Status, Function *F) {
  const DxilFunctionProps &Props = EntryProps.props;
  DXIL::ShaderKind ShaderType = Props.shaderKind;

  ValidateWaveSize(ValCtx, EntryProps, F);

  if (ShaderType == DXIL::ShaderKind::Compute || Props.IsNode()) {
    unsigned X = Props.numThreads[0];
    unsigned Y = Props.numThreads[1];
    unsigned Z = Props.numThreads[2];

    unsigned ThreadsInGroup = X * Y * Z;

    if ((X < DXIL::kMinCSThreadGroupX) || (X > DXIL::kMaxCSThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(X),
                                std::to_string(DXIL::kMinCSThreadGroupX),
                                std::to_string(DXIL::kMaxCSThreadGroupX)});
    }
    if ((Y < DXIL::kMinCSThreadGroupY) || (Y > DXIL::kMaxCSThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(Y),
                                std::to_string(DXIL::kMinCSThreadGroupY),
                                std::to_string(DXIL::kMaxCSThreadGroupY)});
    }
    if ((Z < DXIL::kMinCSThreadGroupZ) || (Z > DXIL::kMaxCSThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(Z),
                                std::to_string(DXIL::kMinCSThreadGroupZ),
                                std::to_string(DXIL::kMaxCSThreadGroupZ)});
    }

    if (ThreadsInGroup > DXIL::kMaxCSThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(ThreadsInGroup),
                                std::to_string(DXIL::kMaxCSThreadsPerGroup)});
    }

    // type of ThreadID, thread group ID take care by DXIL operation overload
    // check.
  } else if (ShaderType == DXIL::ShaderKind::Mesh) {
    const auto &MS = Props.ShaderProps.MS;
    unsigned X = Props.numThreads[0];
    unsigned Y = Props.numThreads[1];
    unsigned Z = Props.numThreads[2];

    unsigned ThreadsInGroup = X * Y * Z;

    if ((X < DXIL::kMinMSASThreadGroupX) || (X > DXIL::kMaxMSASThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(X),
                                std::to_string(DXIL::kMinMSASThreadGroupX),
                                std::to_string(DXIL::kMaxMSASThreadGroupX)});
    }
    if ((Y < DXIL::kMinMSASThreadGroupY) || (Y > DXIL::kMaxMSASThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(Y),
                                std::to_string(DXIL::kMinMSASThreadGroupY),
                                std::to_string(DXIL::kMaxMSASThreadGroupY)});
    }
    if ((Z < DXIL::kMinMSASThreadGroupZ) || (Z > DXIL::kMaxMSASThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(Z),
                                std::to_string(DXIL::kMinMSASThreadGroupZ),
                                std::to_string(DXIL::kMaxMSASThreadGroupZ)});
    }

    if (ThreadsInGroup > DXIL::kMaxMSASThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(ThreadsInGroup),
                                std::to_string(DXIL::kMaxMSASThreadsPerGroup)});
    }

    // type of ThreadID, thread group ID take care by DXIL operation overload
    // check.

    unsigned MaxVertexCount = MS.maxVertexCount;
    if (MaxVertexCount > DXIL::kMaxMSOutputVertexCount) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMeshShaderMaxVertexCount,
                               {std::to_string(DXIL::kMaxMSOutputVertexCount),
                                std::to_string(MaxVertexCount)});
    }

    unsigned MaxPrimitiveCount = MS.maxPrimitiveCount;
    if (MaxPrimitiveCount > DXIL::kMaxMSOutputPrimitiveCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmMeshShaderMaxPrimitiveCount,
          {std::to_string(DXIL::kMaxMSOutputPrimitiveCount),
           std::to_string(MaxPrimitiveCount)});
    }
  } else if (ShaderType == DXIL::ShaderKind::Amplification) {
    unsigned X = Props.numThreads[0];
    unsigned Y = Props.numThreads[1];
    unsigned Z = Props.numThreads[2];

    unsigned ThreadsInGroup = X * Y * Z;

    if ((X < DXIL::kMinMSASThreadGroupX) || (X > DXIL::kMaxMSASThreadGroupX)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"X", std::to_string(X),
                                std::to_string(DXIL::kMinMSASThreadGroupX),
                                std::to_string(DXIL::kMaxMSASThreadGroupX)});
    }
    if ((Y < DXIL::kMinMSASThreadGroupY) || (Y > DXIL::kMaxMSASThreadGroupY)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Y", std::to_string(Y),
                                std::to_string(DXIL::kMinMSASThreadGroupY),
                                std::to_string(DXIL::kMaxMSASThreadGroupY)});
    }
    if ((Z < DXIL::kMinMSASThreadGroupZ) || (Z > DXIL::kMaxMSASThreadGroupZ)) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmThreadGroupChannelRange,
                               {"Z", std::to_string(Z),
                                std::to_string(DXIL::kMinMSASThreadGroupZ),
                                std::to_string(DXIL::kMaxMSASThreadGroupZ)});
    }

    if (ThreadsInGroup > DXIL::kMaxMSASThreadsPerGroup) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmMaxTheadGroup,
                               {std::to_string(ThreadsInGroup),
                                std::to_string(DXIL::kMaxMSASThreadsPerGroup)});
    }

    // type of ThreadID, thread group ID take care by DXIL operation overload
    // check.
  } else if (ShaderType == DXIL::ShaderKind::Domain) {
    const auto &DS = Props.ShaderProps.DS;
    DXIL::TessellatorDomain Domain = DS.domain;
    if (Domain >= DXIL::TessellatorDomain::LastEntry)
      Domain = DXIL::TessellatorDomain::Undefined;
    unsigned InputControlPointCount = DS.inputControlPoints;

    if (InputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmDSInputControlPointCountRange,
          {std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(InputControlPointCount)});
    }
    if (Domain == DXIL::TessellatorDomain::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmValidDomain);
    }
    CheckPatchConstantSemantic(ValCtx, EntryProps, Status, F);
  } else if (ShaderType == DXIL::ShaderKind::Hull) {
    const auto &HS = Props.ShaderProps.HS;
    DXIL::TessellatorDomain Domain = HS.domain;
    if (Domain >= DXIL::TessellatorDomain::LastEntry)
      Domain = DXIL::TessellatorDomain::Undefined;
    unsigned InputControlPointCount = HS.inputControlPoints;
    if (InputControlPointCount == 0) {
      const DxilSignature &InputSig = EntryProps.sig.InputSignature;
      if (!InputSig.GetElements().empty()) {
        ValCtx.EmitFnError(F,
                           ValidationRule::SmZeroHSInputControlPointWithInput);
      }
    } else if (InputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmHSInputControlPointCountRange,
          {std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(InputControlPointCount)});
    }

    unsigned OutputControlPointCount = HS.outputControlPoints;
    if (OutputControlPointCount < DXIL::kMinIAPatchControlPointCount ||
        OutputControlPointCount > DXIL::kMaxIAPatchControlPointCount) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::SmOutputControlPointCountRange,
          {std::to_string(DXIL::kMinIAPatchControlPointCount),
           std::to_string(DXIL::kMaxIAPatchControlPointCount),
           std::to_string(OutputControlPointCount)});
    }
    if (Domain == DXIL::TessellatorDomain::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmValidDomain);
    }
    DXIL::TessellatorPartitioning Partition = HS.partition;
    if (Partition == DXIL::TessellatorPartitioning::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::MetaTessellatorPartition);
    }

    DXIL::TessellatorOutputPrimitive TessOutputPrimitive = HS.outputPrimitive;
    if (TessOutputPrimitive == DXIL::TessellatorOutputPrimitive::Undefined ||
        TessOutputPrimitive == DXIL::TessellatorOutputPrimitive::LastEntry) {
      ValCtx.EmitFnError(F, ValidationRule::MetaTessellatorOutputPrimitive);
    }

    float MaxTessFactor = HS.maxTessFactor;
    if (MaxTessFactor < DXIL::kHSMaxTessFactorLowerBound ||
        MaxTessFactor > DXIL::kHSMaxTessFactorUpperBound) {
      ValCtx.EmitFnFormatError(
          F, ValidationRule::MetaMaxTessFactor,
          {std::to_string(DXIL::kHSMaxTessFactorLowerBound),
           std::to_string(DXIL::kHSMaxTessFactorUpperBound),
           std::to_string(MaxTessFactor)});
    }
    // Domain and OutPrimivtive match.
    switch (Domain) {
    case DXIL::TessellatorDomain::IsoLine:
      switch (TessOutputPrimitive) {
      case DXIL::TessellatorOutputPrimitive::TriangleCW:
      case DXIL::TessellatorOutputPrimitive::TriangleCCW:
        ValCtx.EmitFnError(F, ValidationRule::SmIsoLineOutputPrimitiveMismatch);
        break;
      default:
        break;
      }
      break;
    case DXIL::TessellatorDomain::Tri:
      switch (TessOutputPrimitive) {
      case DXIL::TessellatorOutputPrimitive::Line:
        ValCtx.EmitFnError(F, ValidationRule::SmTriOutputPrimitiveMismatch);
        break;
      default:
        break;
      }
      break;
    case DXIL::TessellatorDomain::Quad:
      switch (TessOutputPrimitive) {
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

    CheckPatchConstantSemantic(ValCtx, EntryProps, Status, F);
  } else if (ShaderType == DXIL::ShaderKind::Geometry) {
    const auto &GS = Props.ShaderProps.GS;
    unsigned MaxVertexCount = GS.maxVertexCount;
    if (MaxVertexCount > DXIL::kMaxGSOutputVertexCount) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmGSOutputVertexCountRange,
                               {std::to_string(DXIL::kMaxGSOutputVertexCount),
                                std::to_string(MaxVertexCount)});
    }

    unsigned InstanceCount = GS.instanceCount;
    if (InstanceCount > DXIL::kMaxGSInstanceCount || InstanceCount < 1) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmGSInstanceCountRange,
                               {std::to_string(DXIL::kMaxGSInstanceCount),
                                std::to_string(InstanceCount)});
    }

    DXIL::PrimitiveTopology Topo = DXIL::PrimitiveTopology::Undefined;
    bool TopoMismatch = false;
    for (size_t I = 0; I < _countof(GS.streamPrimitiveTopologies); ++I) {
      if (GS.streamPrimitiveTopologies[I] !=
          DXIL::PrimitiveTopology::Undefined) {
        if (Topo == DXIL::PrimitiveTopology::Undefined)
          Topo = GS.streamPrimitiveTopologies[I];
        else if (Topo != GS.streamPrimitiveTopologies[I]) {
          TopoMismatch = true;
          break;
        }
      }
    }
    if (TopoMismatch)
      Topo = DXIL::PrimitiveTopology::Undefined;
    switch (Topo) {
    case DXIL::PrimitiveTopology::PointList:
    case DXIL::PrimitiveTopology::LineStrip:
    case DXIL::PrimitiveTopology::TriangleStrip:
      break;
    default: {
      ValCtx.EmitFnError(F, ValidationRule::SmGSValidOutputPrimitiveTopology);
    } break;
    }

    DXIL::InputPrimitive InputPrimitive = GS.inputPrimitive;
    unsigned VertexCount = GetNumVertices(InputPrimitive);
    if (VertexCount == 0 && InputPrimitive != DXIL::InputPrimitive::Undefined) {
      ValCtx.EmitFnError(F, ValidationRule::SmGSValidInputPrimitive);
    }
  }
}

static void ValidateShaderState(ValidationContext &ValCtx) {
  DxilModule &DM = ValCtx.DxilMod;
  if (ValCtx.isLibProfile) {
    for (Function &F : DM.GetModule()->functions()) {
      if (DM.HasDxilEntryProps(&F)) {
        DxilEntryProps &EntryProps = DM.GetDxilEntryProps(&F);
        EntryStatus &Status = ValCtx.GetEntryStatus(&F);
        ValidateEntryProps(ValCtx, EntryProps, Status, &F);
        ValidatePassThruHS(ValCtx, EntryProps, &F);
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
    DxilEntryProps &EntryProps = DM.GetDxilEntryProps(Entry);
    ValidateEntryProps(ValCtx, EntryProps, Status, Entry);
    ValidatePassThruHS(ValCtx, EntryProps, Entry);
  }
}

static CallGraphNode *
CalculateCallDepth(CallGraphNode *Node,
                   std::unordered_map<CallGraphNode *, unsigned> &DepthMap,
                   std::unordered_set<CallGraphNode *> &CallStack,
                   std::unordered_set<Function *> &FuncSet) {
  unsigned Depth = CallStack.size();
  FuncSet.insert(Node->getFunction());
  for (auto It = Node->begin(), EIt = Node->end(); It != EIt; It++) {
    CallGraphNode *ToNode = It->second;
    if (CallStack.insert(ToNode).second == false) {
      // Recursive.
      return ToNode;
    }
    if (DepthMap[ToNode] < Depth)
      DepthMap[ToNode] = Depth;
    if (CallGraphNode *N =
            CalculateCallDepth(ToNode, DepthMap, CallStack, FuncSet)) {
      // Recursive
      return N;
    }
    CallStack.erase(ToNode);
  }

  return nullptr;
}

static void ValidateCallGraph(ValidationContext &ValCtx) {
  // Build CallGraph.
  CallGraph &CG = ValCtx.GetCallGraph();

  std::unordered_map<CallGraphNode *, unsigned> DepthMap;
  std::unordered_set<CallGraphNode *> CallStack;
  CallGraphNode *EntryNode = CG[ValCtx.DxilMod.GetEntryFunction()];
  DepthMap[EntryNode] = 0;
  if (CallGraphNode *N = CalculateCallDepth(EntryNode, DepthMap, CallStack,
                                            ValCtx.entryFuncCallSet))
    ValCtx.EmitFnError(N->getFunction(), ValidationRule::FlowNoRecursion);
  if (ValCtx.DxilMod.GetShaderModel()->IsHS()) {
    CallGraphNode *PatchConstantNode =
        CG[ValCtx.DxilMod.GetPatchConstantFunction()];
    DepthMap[PatchConstantNode] = 0;
    CallStack.clear();
    if (CallGraphNode *N =
            CalculateCallDepth(PatchConstantNode, DepthMap, CallStack,
                               ValCtx.patchConstFuncCallSet))
      ValCtx.EmitFnError(N->getFunction(), ValidationRule::FlowNoRecursion);
  }
}

static void ValidateFlowControl(ValidationContext &ValCtx) {
  bool Reducible =
      IsReducible(*ValCtx.DxilMod.GetModule(), IrreducibilityAction::Ignore);
  if (!Reducible) {
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
    for (auto LoopIt = LI.begin(); LoopIt != LI.end(); LoopIt++) {
      Loop *Loop = *LoopIt;
      SmallVector<BasicBlock *, 4> ExitBlocks;
      Loop->getExitBlocks(ExitBlocks);
      if (ExitBlocks.empty())
        ValCtx.EmitFnError(&F, ValidationRule::FlowDeadLoop);
    }

    // validate that there is no use of a value that has been output-completed
    // for this function.

    hlsl::OP *HlslOP = ValCtx.DxilMod.GetOP();

    for (auto &It : HlslOP->GetOpFuncList(DXIL::OpCode::OutputComplete)) {
      Function *pF = It.second;
      if (!pF)
        continue;

      // first, collect all the output complete calls that are not dominated
      // by another OutputComplete call for the same handle value
      llvm::SmallMapVector<Value *, llvm::SmallPtrSet<CallInst *, 4>, 4>
          HandleToCI;
      for (User *U : pF->users()) {
        // all OutputComplete calls are instructions, and call instructions,
        // so there shouldn't need to be a null check.
        CallInst *CI = cast<CallInst>(U);

        // verify that the function that contains this instruction is the same
        // function that the DominatorTree was built on.
        if (&F != CI->getParent()->getParent())
          continue;

        DxilInst_OutputComplete OutputComplete(CI);
        Value *CompletedRecord = OutputComplete.get_output();

        auto vIt = HandleToCI.find(CompletedRecord);
        if (vIt == HandleToCI.end()) {
          llvm::SmallPtrSet<CallInst *, 4> s;
          s.insert(CI);
          HandleToCI.insert(std::make_pair(CompletedRecord, s));
        } else {
          // if the handle is already in the map, make sure the map's set of
          // output complete calls that dominate the handle and do not dominate
          // each other gets updated if necessary
          bool CI_is_dominated = false;
          for (auto OcIt = vIt->second.begin(); OcIt != vIt->second.end();) {
            // if our new OC CI dominates an OC instruction in the set,
            // then replace the instruction in the set with the new OC CI.

            if (DT.dominates(CI, *OcIt)) {
              auto cur_it = OcIt++;
              vIt->second.erase(*cur_it);
              continue;
            }
            // Remember if our new CI gets dominated by any CI in the set.
            if (DT.dominates(*OcIt, CI)) {
              CI_is_dominated = true;
              break;
            }
            OcIt++;
          }
          // if no CI in the set dominates our new CI,
          // the new CI should be added to the set
          if (!CI_is_dominated)
            vIt->second.insert(CI);
        }
      }

      for (auto handle_iter = HandleToCI.begin(), e = HandleToCI.end();
           handle_iter != e; handle_iter++) {
        for (auto user_itr = handle_iter->first->user_begin();
             user_itr != handle_iter->first->user_end(); user_itr++) {
          User *pU = *user_itr;
          Instruction *UseInstr = cast<Instruction>(pU);
          if (UseInstr) {
            if (CallInst *CI = dyn_cast<CallInst>(UseInstr)) {
              // if the user is an output complete call that is in the set of
              // OutputComplete calls not dominated by another OutputComplete
              // call for the same handle value, no diagnostics need to be
              // emitted.
              if (handle_iter->second.count(CI) == 1)
                continue;
            }

            // make sure any output complete call in the set
            // that dominates this use gets its diagnostic emitted.
            for (auto OcIt = handle_iter->second.begin();
                 OcIt != handle_iter->second.end(); OcIt++) {
              Instruction *OcInstr = cast<Instruction>(*OcIt);
              if (DT.dominates(OcInstr, UseInstr)) {
                ValCtx.EmitInstrError(
                    UseInstr,
                    ValidationRule::InstrNodeRecordHandleUseAfterComplete);
                ValCtx.EmitInstrNote(
                    *OcIt, "record handle invalidated by OutputComplete");
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
  DxilEntryProps &EntryProps = DM.GetDxilEntryProps(F);
  EntryStatus &Status = ValCtx.GetEntryStatus(F);
  const DxilFunctionProps &Props = EntryProps.props;
  // For HS only need to check Tessfactor which is in patch constant sig.
  if (Props.IsHS()) {
    std::vector<unsigned> &PatchConstOrPrimCols = Status.patchConstOrPrimCols;
    const DxilSignature &PatchConstSig =
        EntryProps.sig.PatchConstOrPrimSignature;
    for (auto &E : PatchConstSig.GetElements()) {
      unsigned Mask = PatchConstOrPrimCols[E->GetID()];
      unsigned RequireMask = (1 << E->GetCols()) - 1;
      // TODO: check other case uninitialized output is allowed.
      if (Mask != RequireMask && !E->GetSemantic()->IsArbitrary()) {
        ValCtx.EmitFnFormatError(F, ValidationRule::SmUndefinedOutput,
                                 {E->GetName()});
      }
    }
    return;
  }
  const DxilSignature &OutSig = EntryProps.sig.OutputSignature;
  std::vector<unsigned> &OutputCols = Status.outputCols;
  for (auto &E : OutSig.GetElements()) {
    unsigned Mask = OutputCols[E->GetID()];
    unsigned RequireMask = (1 << E->GetCols()) - 1;
    // TODO: check other case uninitialized output is allowed.
    if (Mask != RequireMask && !E->GetSemantic()->IsArbitrary() &&
        E->GetSemantic()->GetKind() != Semantic::Kind::Target) {
      ValCtx.EmitFnFormatError(F, ValidationRule::SmUndefinedOutput,
                               {E->GetName()});
    }
  }

  if (!Props.IsGS()) {
    unsigned PosMask = Status.OutputPositionMask[0];
    if (PosMask != 0xf && Status.hasOutputPosition[0]) {
      ValCtx.EmitFnError(F, ValidationRule::SmCompletePosition);
    }
  } else {
    const auto &GS = Props.ShaderProps.GS;
    unsigned StreamMask = 0;
    for (size_t I = 0; I < _countof(GS.streamPrimitiveTopologies); ++I) {
      if (GS.streamPrimitiveTopologies[I] !=
          DXIL::PrimitiveTopology::Undefined) {
        StreamMask |= 1 << I;
      }
    }

    for (unsigned I = 0; I < DXIL::kNumOutputStreams; I++) {
      if (StreamMask & (1 << I)) {
        unsigned PosMask = Status.OutputPositionMask[I];
        if (PosMask != 0xf && Status.hasOutputPosition[I]) {
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
