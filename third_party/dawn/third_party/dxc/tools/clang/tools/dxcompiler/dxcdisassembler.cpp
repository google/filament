///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcdisassembler.cpp                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements Disassembler.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilPDB.h"
#include "dxc/DXIL/DxilResource.h"
#include "dxc/DXIL/DxilResourceProperties.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/HLSL/ComputeViewIdState.h"
#include "dxc/HLSL/HLMatrixType.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxcutil.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormattedStream.h"
#include <assert.h> // Needed for DxilPipelineStateValidation.h

using namespace llvm;
using namespace hlsl;
using namespace hlsl::DXIL;

namespace {
// Disassemble helper functions.

template <typename T> const T *ByteOffset(LPCVOID p, uint32_t byteOffset) {
  return reinterpret_cast<const T *>((const uint8_t *)p + byteOffset);
}
bool SigElementHasStream(const DxilProgramSignatureElement &pSignature) {
  return pSignature.Stream != 0;
}

void PrintSignature(LPCSTR pName, const DxilProgramSignature *pSignature,
                    bool bIsInput, raw_string_ostream &OS, StringRef comment) {
  OS << comment << "\n"
     << comment << " " << pName << " signature:\n"
     << comment << "\n"
     << comment
     << " Name                 Index   Mask Register SysValue  Format   Used\n"
     << comment
     << " -------------------- ----- ------ -------- -------- ------- ------\n";

  if (pSignature->ParamCount == 0) {
    OS << comment << " no parameters\n";
    return;
  }

  const DxilProgramSignatureElement *pSigBegin =
      ByteOffset<DxilProgramSignatureElement>(pSignature,
                                              pSignature->ParamOffset);
  const DxilProgramSignatureElement *pSigEnd =
      pSigBegin + pSignature->ParamCount;

  bool bHasStreams = std::any_of(pSigBegin, pSigEnd, SigElementHasStream);
  for (const DxilProgramSignatureElement *pSig = pSigBegin; pSig != pSigEnd;
       ++pSig) {
    OS << comment << " ";
    const char *pSemanticName =
        ByteOffset<char>(pSignature, pSig->SemanticName);
    if (bHasStreams) {
      OS << "m" << pSig->Stream << ":";
      OS << left_justify(pSemanticName, 17);
    } else {
      OS << left_justify(pSemanticName, 20);
    }

    OS << ' ' << format("%5u", pSig->SemanticIndex);

    char Mask[4];
    memset(Mask, ' ', sizeof(Mask));

    if (pSig->Mask & DxilProgramSigMaskX)
      Mask[0] = 'x';
    if (pSig->Mask & DxilProgramSigMaskY)
      Mask[1] = 'y';
    if (pSig->Mask & DxilProgramSigMaskZ)
      Mask[2] = 'z';
    if (pSig->Mask & DxilProgramSigMaskW)
      Mask[3] = 'w';

    if (pSig->Register == (unsigned)-1) {
      OS << "    N/A";
      if (!_stricmp(pSemanticName, "SV_Depth"))
        OS << "   oDepth";
      else if (0 == _stricmp(pSemanticName, "SV_DepthGreaterEqual"))
        OS << " oDepthGE";
      else if (0 == _stricmp(pSemanticName, "SV_DepthLessEqual"))
        OS << " oDepthLE";
      else if (0 == _stricmp(pSemanticName, "SV_Coverage"))
        OS << "    oMask";
      else if (0 == _stricmp(pSemanticName, "SV_StencilRef"))
        OS << "    oStencilRef";
      else if (pSig->SystemValue == DxilProgramSigSemantic::PrimitiveID)
        OS << "   primID";
      else
        OS << "  special";
    } else {
      OS << "   " << Mask[0] << Mask[1] << Mask[2] << Mask[3];
      OS << ' ' << format("%8u", pSig->Register);
    }

    LPCSTR pSysValue = "NONE";
    switch (pSig->SystemValue) {
    case DxilProgramSigSemantic::ClipDistance:
      pSysValue = "CLIPDST";
      break;
    case DxilProgramSigSemantic::CullDistance:
      pSysValue = "CULLDST";
      break;
    case DxilProgramSigSemantic::Position:
      pSysValue = "POS";
      break;
    case DxilProgramSigSemantic::RenderTargetArrayIndex:
      pSysValue = "RTINDEX";
      break;
    case DxilProgramSigSemantic::ViewPortArrayIndex:
      pSysValue = "VPINDEX";
      break;
    case DxilProgramSigSemantic::VertexID:
      pSysValue = "VERTID";
      break;
    case DxilProgramSigSemantic::PrimitiveID:
      pSysValue = "PRIMID";
      break;
    case DxilProgramSigSemantic::InstanceID:
      pSysValue = "INSTID";
      break;
    case DxilProgramSigSemantic::IsFrontFace:
      pSysValue = "FFACE";
      break;
    case DxilProgramSigSemantic::SampleIndex:
      pSysValue = "SAMPLE";
      break;
    case DxilProgramSigSemantic::Target:
      pSysValue = "TARGET";
      break;
    case DxilProgramSigSemantic::Depth:
      pSysValue = "DEPTH";
      break;
    case DxilProgramSigSemantic::DepthGE:
      pSysValue = "DEPTHGE";
      break;
    case DxilProgramSigSemantic::DepthLE:
      pSysValue = "DEPTHLE";
      break;
    case DxilProgramSigSemantic::Coverage:
      pSysValue = "COVERAGE";
      break;
    case DxilProgramSigSemantic::InnerCoverage:
      pSysValue = "INNERCOV";
      break;
    case DxilProgramSigSemantic::StencilRef:
      pSysValue = "STENCILREF";
      break;
    case DxilProgramSigSemantic::FinalQuadEdgeTessfactor:
      pSysValue = "QUADEDGE";
      break;
    case DxilProgramSigSemantic::FinalQuadInsideTessfactor:
      pSysValue = "QUADINT";
      break;
    case DxilProgramSigSemantic::FinalTriEdgeTessfactor:
      pSysValue = "TRIEDGE";
      break;
    case DxilProgramSigSemantic::FinalTriInsideTessfactor:
      pSysValue = "TRIINT";
      break;
    case DxilProgramSigSemantic::FinalLineDetailTessfactor:
      pSysValue = "LINEDET";
      break;
    case DxilProgramSigSemantic::FinalLineDensityTessfactor:
      pSysValue = "LINEDEN";
      break;
    case DxilProgramSigSemantic::Barycentrics:
      pSysValue = "BARYCEN";
      break;
    case DxilProgramSigSemantic::ShadingRate:
      pSysValue = "SHDINGRATE";
      break;
    case DxilProgramSigSemantic::CullPrimitive:
      pSysValue = "CULLPRIM";
      break;
    case DxilProgramSigSemantic::Undefined:
      break;
    }
    OS << right_justify(pSysValue, 9);

    LPCSTR pFormat = "unknown";
    switch (pSig->CompType) {
    case DxilProgramSigCompType::Float32:
      pFormat = "float";
      break;
    case DxilProgramSigCompType::SInt32:
      pFormat = "int";
      break;
    case DxilProgramSigCompType::UInt32:
      pFormat = "uint";
      break;
    case DxilProgramSigCompType::UInt16:
      pFormat = "uint16";
      break;
    case DxilProgramSigCompType::SInt16:
      pFormat = "int16";
      break;
    case DxilProgramSigCompType::Float16:
      pFormat = "fp16";
      break;
    case DxilProgramSigCompType::UInt64:
      pFormat = "uint64";
      break;
    case DxilProgramSigCompType::SInt64:
      pFormat = "int64";
      break;
    case DxilProgramSigCompType::Float64:
      pFormat = "double";
      break;
    case DxilProgramSigCompType::Unknown:
      break;
    }

    OS << right_justify(pFormat, 8);

    memset(Mask, ' ', sizeof(Mask));

    BYTE rwMask = pSig->AlwaysReads_Mask;
    if (!bIsInput)
      rwMask = ~rwMask;

    if (rwMask & DxilProgramSigMaskX)
      Mask[0] = 'x';
    if (rwMask & DxilProgramSigMaskY)
      Mask[1] = 'y';
    if (rwMask & DxilProgramSigMaskZ)
      Mask[2] = 'z';
    if (rwMask & DxilProgramSigMaskW)
      Mask[3] = 'w';

    if (pSig->Register == (unsigned)-1)
      OS << (rwMask ? "    YES" : "     NO");
    else
      OS << "   " << Mask[0] << Mask[1] << Mask[2] << Mask[3];

    OS << "\n";
  }
  OS << comment << "\n";
}

void PintCompMaskNameCompact(raw_string_ostream &OS, unsigned CompMask) {
  char Mask[5];
  memset(Mask, '\0', sizeof(Mask));
  unsigned idx = 0;
  if (CompMask & DxilProgramSigMaskX)
    Mask[idx++] = 'x';
  if (CompMask & DxilProgramSigMaskY)
    Mask[idx++] = 'y';
  if (CompMask & DxilProgramSigMaskZ)
    Mask[idx++] = 'z';
  if (CompMask & DxilProgramSigMaskW)
    Mask[idx++] = 'w';
  OS << right_justify(Mask, 4);
}

void PrintDxilSignature(LPCSTR pName, const DxilSignature &Signature,
                        raw_string_ostream &OS, StringRef comment) {
  const std::vector<std::unique_ptr<DxilSignatureElement>> &sigElts =
      Signature.GetElements();
  if (sigElts.size() == 0)
    return;
  // TODO: Print all the data in DxilSignature.
  OS << comment << "\n"
     << comment << " " << pName << " signature:\n"
     << comment << "\n"
     << comment << " Name                 Index             InterpMode DynIdx\n"
     << comment
     << " -------------------- ----- ---------------------- ------\n";

  for (auto &sigElt : sigElts) {
    OS << comment << " ";

    OS << left_justify(sigElt->GetName(), 20);
    auto &indexVec = sigElt->GetSemanticIndexVec();
    unsigned index = 0;
    if (!indexVec.empty()) {
      index = sigElt->GetSemanticIndexVec()[0];
    }
    OS << ' ' << format("%5u", index);
    sigElt->GetInterpolationMode()->GetName();
    OS << ' ' << right_justify(sigElt->GetInterpolationMode()->GetName(), 22);
    OS << "   ";
    PintCompMaskNameCompact(OS, sigElt->GetDynIdxCompMask());
    OS << "\n";
  }
}

PCSTR g_pFeatureInfoNames[] = {
    "Double-precision floating point",
    "Raw and Structured buffers",
    "UAVs at every shader stage",
    "64 UAV slots",
    "Minimum-precision data types",
    "Double-precision extensions for 11.1",
    "Shader extensions for 11.1",
    "Comparison filtering for feature level 9",
    "Tiled resources",
    "PS Output Stencil Ref",
    "PS Inner Coverage",
    "Typed UAV Load Additional Formats",
    "Raster Ordered UAVs",
    ("SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader "
     "feeding rasterizer"),
    "Wave level operations",
    "64-Bit integer",
    "View Instancing",
    "Barycentrics",
    "Use native low precision",
    "Shading Rate",
    "Raytracing tier 1.1 features",
    "Sampler feedback",
    "64-bit Atomics on Typed Resources",
    "64-bit Atomics on Group Shared",
    "Derivatives in mesh and amplification shaders",
    "Resource descriptor heap indexing",
    "Sampler descriptor heap indexing",
    "Wave Matrix",
    "64-bit Atomics on Heap Resources",
    "Advanced Texture Ops",
    "Writeable MSAA Textures",
    "SampleCmp with gradient or bias",
    "Extended command info",
};
static_assert(_countof(g_pFeatureInfoNames) == ShaderFeatureInfoCount,
              "g_pFeatureInfoNames needs to be updated");

PCSTR g_pOptFeatureInfoNames[] = {
    "Function uses derivatives",
    "Function requires visible group",
};
static_assert(_countof(g_pOptFeatureInfoNames) == OptFeatureInfoCount,
              "g_pOptFeatureInfoNames needs to be updated");

void PrintFeatureInfo(const DxilShaderFeatureInfo *pFeatureInfo,
                      raw_string_ostream &OS, StringRef comment) {
  uint64_t featureFlags = pFeatureInfo->FeatureFlags;
  if (!featureFlags)
    return;
  OS << comment << "\n";
  OS << comment << " Note: shader requires additional functionality:\n";
  for (unsigned i = 0; i < ShaderFeatureInfoCount; i++) {
    if (featureFlags & (((uint64_t)1) << i))
      OS << comment << "       " << g_pFeatureInfoNames[i] << "\n";
  }
  OS << comment << "\n";

  uint64_t optFeatureFlags = featureFlags >> OptFeatureInfoShift;
  if (!optFeatureFlags)
    return;
  OS << comment << " Note: shader has optional feature flags set:\n";
  for (unsigned i = 0; i < OptFeatureInfoCount; i++) {
    if (optFeatureFlags & (((uint64_t)1) << i))
      OS << comment << "       " << g_pOptFeatureInfoNames[i] << "\n";
  }
  OS << comment << "\n";
}

void PrintResourceFormat(DxilResourceBase &res, unsigned alignment,
                         raw_string_ostream &OS) {
  switch (res.GetClass()) {
  case DxilResourceBase::Class::CBuffer:
  case DxilResourceBase::Class::Sampler:
    OS << right_justify("NA", alignment);
    break;
  case DxilResourceBase::Class::UAV:
  case DxilResourceBase::Class::SRV:
    switch (res.GetKind()) {
    case DxilResource::Kind::RawBuffer:
      OS << right_justify("byte", alignment);
      break;
    case DxilResource::Kind::StructuredBuffer:
      OS << right_justify("struct", alignment);
      break;
    default:
      DxilResource *pRes = static_cast<DxilResource *>(&res);
      CompType &&compType = pRes->GetCompType();
      const char *compName = compType.GetName();
      // TODO: add vector size.
      OS << right_justify(compName, alignment);
      break;
    }
    break;
  case DxilResource::Class::Invalid:
    break;
  }
}

void PrintResourceDim(DxilResourceBase &res, unsigned alignment,
                      raw_string_ostream &OS) {
  switch (res.GetClass()) {
  case DxilResourceBase::Class::CBuffer:
  case DxilResourceBase::Class::Sampler:
    OS << right_justify("NA", alignment);
    break;
  case DxilResourceBase::Class::UAV:
  case DxilResourceBase::Class::SRV:
    switch (res.GetKind()) {
    case DxilResource::Kind::RawBuffer:
    case DxilResource::Kind::StructuredBuffer:
      if (res.GetClass() == DxilResourceBase::Class::SRV)
        OS << right_justify("r/o", alignment);
      else {
        DxilResource &dxilRes = static_cast<DxilResource &>(res);
        if (!dxilRes.HasCounter())
          OS << right_justify("r/w", alignment);
        else
          OS << right_justify("r/w+cnt", alignment);
      }
      break;
    case DxilResource::Kind::TypedBuffer:
      OS << right_justify("buf", alignment);
      break;
    case DxilResource::Kind::Texture2DMS:
    case DxilResource::Kind::Texture2DMSArray: {
      DxilResource *pRes = static_cast<DxilResource *>(&res);
      std::string dimName = res.GetResDimName();
      if (pRes->GetSampleCount())
        dimName += std::to_string(pRes->GetSampleCount());
      OS << right_justify(dimName, alignment);
    } break;
    default:
      OS << right_justify(res.GetResDimName(), alignment);
      break;
    }
    break;
  case DxilResourceBase::Class::Invalid:
    break;
  }
}

void PrintResourceBinding(DxilResourceBase &res, raw_string_ostream &OS,
                          StringRef comment) {
  OS << comment << " " << left_justify(res.GetGlobalName(), 31);

  OS << right_justify(res.GetResClassName(), 10);

  PrintResourceFormat(res, 8, OS);

  PrintResourceDim(res, 12, OS);

  std::string ID = res.GetResIDPrefix();
  ID += std::to_string(res.GetID());
  OS << right_justify(ID, 8);

  std::string bind = res.GetResBindPrefix();
  bind += std::to_string(res.GetLowerBound());
  if (res.GetSpaceID())
    bind += ",space" + std::to_string(res.GetSpaceID());

  OS << right_justify(bind, 15);
  if (res.GetRangeSize() != UINT_MAX)
    OS << right_justify(std::to_string(res.GetRangeSize()), 6) << "\n";
  else
    OS << right_justify("unbounded", 6) << "\n";
}

void PrintResourceBindings(DxilModule &M, raw_string_ostream &OS,
                           StringRef comment) {
  OS << comment << "\n"
     << comment << " Resource Bindings:\n"
     << comment << "\n"
     << comment
     << " Name                                 Type  Format         Dim      "
        "ID      HLSL Bind  Count\n"
     << comment
     << " ------------------------------ ---------- ------- ----------- "
        "------- -------------- ------\n";

  for (auto &res : M.GetCBuffers()) {
    PrintResourceBinding(*res.get(), OS, comment);
  }
  for (auto &res : M.GetSamplers()) {
    PrintResourceBinding(*res.get(), OS, comment);
  }
  for (auto &res : M.GetSRVs()) {
    PrintResourceBinding(*res.get(), OS, comment);
  }
  for (auto &res : M.GetUAVs()) {
    PrintResourceBinding(*res.get(), OS, comment);
  }
  OS << comment << "\n";
}

void PrintOutputsDependentOnViewId(
    llvm::raw_ostream &OS, llvm::StringRef comment, llvm::StringRef SetName,
    unsigned NumOutputs,
    const DxilViewIdState::OutputsDependentOnViewIdType
        &OutputsDependentOnViewId) {
  OS << comment << " " << SetName << " dependent on ViewId: { ";
  bool bFirst = true;
  for (unsigned i = 0; i < NumOutputs; i++) {
    if (OutputsDependentOnViewId[i]) {
      if (!bFirst)
        OS << ", ";
      OS << i;
      bFirst = false;
    }
  }
  OS << " }\n";
}

void PrintInputsContributingToOutputs(
    llvm::raw_ostream &OS, llvm::StringRef comment,
    llvm::StringRef InputSetName, llvm::StringRef OutputSetName,
    const DxilViewIdState::InputsContributingToOutputType
        &InputsContributingToOutputs) {
  OS << comment << " " << InputSetName << " contributing to computation of "
     << OutputSetName << ":\n";
  for (auto &it : InputsContributingToOutputs) {
    unsigned outIdx = it.first;
    auto &Inputs = it.second;
    OS << comment << "   output " << outIdx << " depends on inputs: { ";
    bool bFirst = true;
    for (unsigned i : Inputs) {
      if (!bFirst)
        OS << ", ";
      OS << i;
      bFirst = false;
    }
    OS << " }\n";
  }
}

void PrintViewIdState(DxilModule &M, raw_string_ostream &OS,
                      StringRef comment) {
  if (!M.GetModule()->getNamedMetadata("dx.viewIdState"))
    return;

  const ShaderModel *pSM = M.GetShaderModel();

  DxilViewIdState VID(&M);
  auto &SerializedVID = M.GetSerializedViewIdState();
  VID.Deserialize(SerializedVID.data(), SerializedVID.size());
  OS << comment << "\n";
  OS << comment << " ViewId state:\n";
  OS << comment << "\n";
  OS << comment << " Number of inputs: " << VID.getNumInputSigScalars();
  if (!pSM->IsGS()) {
    OS << ", outputs: " << VID.getNumOutputSigScalars(0);
  } else {
    OS << ", outputs per stream: { " << VID.getNumOutputSigScalars(0) << ", "
       << VID.getNumOutputSigScalars(1) << ", " << VID.getNumOutputSigScalars(2)
       << ", " << VID.getNumOutputSigScalars(3) << " }";
  }
  if (pSM->IsHS() || pSM->IsDS()) {
    OS << ", patchconst: " << VID.getNumPCSigScalars();
  } else if (pSM->IsMS()) {
    OS << ", primitive outputs: " << VID.getNumPCSigScalars();
  }
  OS << "\n";

  if (!pSM->IsGS()) {
    PrintOutputsDependentOnViewId(OS, comment, "Outputs",
                                  VID.getNumOutputSigScalars(0),
                                  VID.getOutputsDependentOnViewId(0));
  } else {
    for (unsigned i = 0; i < 4; i++) {
      if (VID.getNumOutputSigScalars(i) > 0) {
        std::string OutputsName =
            std::string("Outputs for Stream ") + std::to_string(i);
        PrintOutputsDependentOnViewId(OS, comment, OutputsName,
                                      VID.getNumOutputSigScalars(i),
                                      VID.getOutputsDependentOnViewId(i));
      }
    }
  }
  if (pSM->IsHS()) {
    PrintOutputsDependentOnViewId(OS, comment, "PCOutputs",
                                  VID.getNumPCSigScalars(),
                                  VID.getPCOutputsDependentOnViewId());
  } else if (pSM->IsMS()) {
    PrintOutputsDependentOnViewId(OS, comment, "Primitive Outputs",
                                  VID.getNumPCSigScalars(),
                                  VID.getPCOutputsDependentOnViewId());
  }

  if (!pSM->IsGS()) {
    PrintInputsContributingToOutputs(OS, comment, "Inputs", "Outputs",
                                     VID.getInputsContributingToOutputs(0));
  } else {
    for (unsigned i = 0; i < 4; i++) {
      if (VID.getNumOutputSigScalars(i) > 0) {
        std::string OutputsName =
            std::string("Outputs for Stream ") + std::to_string(i);
        PrintInputsContributingToOutputs(OS, comment, "Inputs", OutputsName,
                                         VID.getInputsContributingToOutputs(i));
      }
    }
  }
  if (pSM->IsHS()) {
    PrintInputsContributingToOutputs(OS, comment, "Inputs", "PCOutputs",
                                     VID.getInputsContributingToPCOutputs());
  } else if (pSM->IsMS()) {
    PrintInputsContributingToOutputs(OS, comment, "Inputs", "Primitive Outputs",
                                     VID.getInputsContributingToPCOutputs());
  } else if (pSM->IsDS()) {
    PrintInputsContributingToOutputs(OS, comment, "PCInputs", "Outputs",
                                     VID.getPCInputsContributingToOutputs());
  }
  OS << comment << "\n";
}

static const char *SubobjectKindToString(DXIL::SubobjectKind kind) {
  switch (kind) {
  case DXIL::SubobjectKind::StateObjectConfig:
    return "StateObjectConfig";
  case DXIL::SubobjectKind::GlobalRootSignature:
    return "GlobalRootSignature";
  case DXIL::SubobjectKind::LocalRootSignature:
    return "LocalRootSignature";
  case DXIL::SubobjectKind::SubobjectToExportsAssociation:
    return "SubobjectToExportsAssociation";
  case DXIL::SubobjectKind::RaytracingShaderConfig:
    return "RaytracingShaderConfig";
  case DXIL::SubobjectKind::RaytracingPipelineConfig:
    return "RaytracingPipelineConfig";
  case DXIL::SubobjectKind::HitGroup:
    return "HitGroup";
  case DXIL::SubobjectKind::RaytracingPipelineConfig1:
    return "RaytracingPipelineConfig1";
  }
  return "<invalid kind>";
}

static const char *FlagToString(DXIL::StateObjectFlags Flag) {
  switch (Flag) {
  case DXIL::StateObjectFlags::AllowLocalDependenciesOnExternalDefinitions:
    return "STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS";
  case DXIL::StateObjectFlags::AllowExternalDependenciesOnLocalDefinitions:
    return "STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS";
  case DXIL::StateObjectFlags::AllowStateObjectAdditions:
    return "STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS";
  }
  return "<invalid StateObjectFlag>";
}

static const char *FlagToString(DXIL::RaytracingPipelineFlags Flag) {
  switch (Flag) {
  case DXIL::RaytracingPipelineFlags::None:
    return "RAYTRACING_PIPELINE_FLAG_NONE";
  case DXIL::RaytracingPipelineFlags::SkipTriangles:
    return "RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES";
  case DXIL::RaytracingPipelineFlags::SkipProceduralPrimitives:
    return "RAYTRACING_PIPELINE_FLAG_SKIP_PROCEDURAL_PRIMITIVES";
  case DXIL::RaytracingPipelineFlags::AllowOpacityMicromaps:
    return "RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS";
  }
  return "<invalid RaytracingPipelineFlags>";
}

static const char *HitGroupTypeToString(DXIL::HitGroupType type) {
  switch (type) {
  case DXIL::HitGroupType::Triangle:
    return "Triangle";
  case DXIL::HitGroupType::ProceduralPrimitive:
    return "ProceduralPrimitive";
  }
  return "<invalid HitGroupType>";
}

template <typename _T> void PrintFlags(raw_string_ostream &OS, uint32_t Flags) {
  if (!Flags) {
    OS << "0";
    return;
  }
  uint32_t flag = 0;
  while (Flags) {
    if (flag)
      OS << " | ";
    flag = (Flags & ~(Flags - 1));
    Flags ^= flag;
    OS << FlagToString((_T)flag);
  }
}

void PrintSubobjects(const DxilSubobjects &subobjects, raw_string_ostream &OS,
                     StringRef comment) {
  if (subobjects.GetSubobjects().empty())
    return;

  OS << comment << "\n";
  OS << comment << " Subobjects:\n";
  OS << comment << "\n";

  for (auto &it : subobjects.GetSubobjects()) {
    StringRef name = it.first;
    if (!it.second) {
      OS << comment << "  " << name << " = <null>"
         << "\n";
      continue;
    }
    const DxilSubobject &obj = *it.second.get();
    OS << comment << "  " << SubobjectKindToString(obj.GetKind()) << " " << name
       << " = "
       << "{ ";
    bool bLocalRS = false;
    switch (obj.GetKind()) {
    case DXIL::SubobjectKind::StateObjectConfig: {
      uint32_t Flags = 0;
      if (!obj.GetStateObjectConfig(Flags)) {
        OS << "<error getting subobject>";
        break;
      }
      PrintFlags<DXIL::StateObjectFlags>(OS, Flags);
      break;
    }
    case DXIL::SubobjectKind::LocalRootSignature:
      bLocalRS = true;
      LLVM_FALLTHROUGH;
    case DXIL::SubobjectKind::GlobalRootSignature: {
      const char *Text = nullptr;
      const void *Data = nullptr;
      uint32_t Size = 0;
      if (!obj.GetRootSignature(bLocalRS, Data, Size, &Text)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "<" << Size << " bytes>";
      if (Text && Text[0]) {
        OS << ", \"" << Text << "\"";
      }
      break;
    }
    case DXIL::SubobjectKind::SubobjectToExportsAssociation: {
      llvm::StringRef Subobject;
      const char *const *Exports = nullptr;
      uint32_t NumExports;
      if (!obj.GetSubobjectToExportsAssociation(Subobject, Exports,
                                                NumExports)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "\"" << Subobject << "\", { ";
      if (Exports) {
        for (unsigned i = 0; i < NumExports; ++i) {
          OS << (i ? ", " : "") << "\"" << Exports[i] << "\"";
        }
      }
      OS << " } ";
      break;
    }
    case DXIL::SubobjectKind::RaytracingShaderConfig: {
      uint32_t MaxPayloadSizeInBytes;
      uint32_t MaxAttributeSizeInBytes;
      if (!obj.GetRaytracingShaderConfig(MaxPayloadSizeInBytes,
                                         MaxAttributeSizeInBytes)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "MaxPayloadSizeInBytes = " << MaxPayloadSizeInBytes
         << ", MaxAttributeSizeInBytes = " << MaxAttributeSizeInBytes;
      break;
    }
    case DXIL::SubobjectKind::RaytracingPipelineConfig: {
      uint32_t MaxTraceRecursionDepth;
      if (!obj.GetRaytracingPipelineConfig(MaxTraceRecursionDepth)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "MaxTraceRecursionDepth = " << MaxTraceRecursionDepth;
      break;
    }
    case DXIL::SubobjectKind::HitGroup: {
      HitGroupType hgType;
      StringRef AnyHit;
      StringRef ClosestHit;
      StringRef Intersection;
      if (!obj.GetHitGroup(hgType, AnyHit, ClosestHit, Intersection)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "HitGroupType = " << HitGroupTypeToString(hgType) << ", Anyhit = \""
         << AnyHit << "\", Closesthit = \"" << ClosestHit
         << "\", Intersection = \"" << Intersection << "\"";
      break;
    }
    case DXIL::SubobjectKind::RaytracingPipelineConfig1: {
      uint32_t MaxTraceRecursionDepth;
      uint32_t Flags;
      if (!obj.GetRaytracingPipelineConfig1(MaxTraceRecursionDepth, Flags)) {
        OS << "<error getting subobject>";
        break;
      }
      OS << "MaxTraceRecursionDepth = " << MaxTraceRecursionDepth;
      OS << ", Flags = ";
      PrintFlags<DXIL::RaytracingPipelineFlags>(OS, Flags);
      break;
    }
    }
    OS << " };\n";
  }
  OS << comment << "\n";
}

void PrintStructLayout(StructType *ST, DxilTypeSystem &typeSys,
                       const DataLayout *DL, raw_string_ostream &OS,
                       StringRef comment, StringRef varName, unsigned offset,
                       unsigned indent, unsigned arraySize,
                       unsigned sizeOfStruct = 0);

void PrintTypeAndName(llvm::Type *Ty, DxilFieldAnnotation &annotation,
                      std::string &StreamStr, unsigned arraySize,
                      bool minPrecision) {
  raw_string_ostream Stream(StreamStr);
  while (Ty->isArrayTy())
    Ty = Ty->getArrayElementType();

  const char *compTyName = annotation.GetCompType().GetHLSLName(minPrecision);
  if (annotation.HasMatrixAnnotation()) {
    const DxilMatrixAnnotation &Matrix = annotation.GetMatrixAnnotation();
    switch (Matrix.Orientation) {
    case MatrixOrientation::RowMajor:
      Stream << "row_major ";
      break;
    case MatrixOrientation::ColumnMajor:
      Stream << "column_major ";
      break;
    case MatrixOrientation::Undefined:
    case MatrixOrientation::LastEntry:
      break;
    }
    Stream << compTyName << Matrix.Rows << "x" << Matrix.Cols;
  } else if (Ty->isVectorTy())
    Stream << compTyName << Ty->getVectorNumElements();
  else
    Stream << compTyName;

  Stream << " " << annotation.GetFieldName();
  if (arraySize)
    Stream << "[" << arraySize << "]";
  Stream << ";";
  Stream.flush();
}

void PrintFieldLayout(llvm::Type *Ty, DxilFieldAnnotation &annotation,
                      DxilTypeSystem &typeSys, const DataLayout *DL,
                      raw_string_ostream &OS, StringRef comment,
                      unsigned offset, unsigned indent, unsigned offsetIndent,
                      unsigned sizeToPrint = 0) {
  if (Ty->isStructTy() && !annotation.HasMatrixAnnotation()) {
    PrintStructLayout(cast<StructType>(Ty), typeSys, DL, OS, comment,
                      annotation.GetFieldName(), offset, indent, offsetIndent);
  } else {
    llvm::Type *EltTy = Ty;
    unsigned arraySize = 0;
    unsigned arrayLevel = 0;
    if (!HLMatrixType::isa(EltTy) && EltTy->isArrayTy()) {
      arraySize = 1;
      while (!HLMatrixType::isa(EltTy) && EltTy->isArrayTy()) {
        arraySize *= EltTy->getArrayNumElements();
        EltTy = EltTy->getArrayElementType();
        arrayLevel++;
      }
    }

    if (annotation.HasMatrixAnnotation()) {
      const DxilMatrixAnnotation &Matrix = annotation.GetMatrixAnnotation();
      switch (Matrix.Orientation) {
      case MatrixOrientation::RowMajor:
        arraySize /= Matrix.Rows;
        break;
      case MatrixOrientation::ColumnMajor:
        arraySize /= Matrix.Cols;
        break;
      case MatrixOrientation::Undefined:
      case MatrixOrientation::LastEntry:
        break;
      }
      if (EltTy->isVectorTy()) {
        EltTy = EltTy->getVectorElementType();
      } else if (EltTy->isStructTy())
        EltTy = HLMatrixType::cast(EltTy).getElementTypeForReg();

      if (arrayLevel == 1)
        arraySize = 0;
    }

    std::string StreamStr;
    if (!HLMatrixType::isa(EltTy) && EltTy->isStructTy()) {
      std::string NameTypeStr = annotation.GetFieldName();
      raw_string_ostream Stream(NameTypeStr);
      if (arraySize)
        Stream << "[" << std::to_string(arraySize) << "]";
      Stream << ";";
      Stream.flush();

      PrintStructLayout(cast<StructType>(EltTy), typeSys, DL, OS, comment,
                        NameTypeStr, offset, indent, offsetIndent);
    } else {
      (OS << comment).indent(indent);
      std::string NameTypeStr;
      PrintTypeAndName(Ty, annotation, NameTypeStr, arraySize,
                       typeSys.UseMinPrecision());
      OS << left_justify(NameTypeStr, offsetIndent);

      // Offset
      OS << comment << " Offset:" << right_justify(std::to_string(offset), 5);
      if (sizeToPrint)
        OS << " Size: " << right_justify(std::to_string(sizeToPrint), 5);
      OS << "\n";
    }
  }
}

// null DataLayout => assume constant buffer layout
void PrintStructLayout(StructType *ST, DxilTypeSystem &typeSys,
                       const DataLayout *DL, raw_string_ostream &OS,
                       StringRef comment, StringRef varName, unsigned offset,
                       unsigned indent, unsigned offsetIndent,
                       unsigned sizeOfStruct) {
  DxilStructAnnotation *annotation = typeSys.GetStructAnnotation(ST);
  (OS << comment).indent(indent) << "struct " << ST->getName() << "\n";
  (OS << comment).indent(indent) << "{\n";
  OS << comment << "\n";

  unsigned fieldIndent = indent + 4;

  if (!annotation) {
    if (!sizeOfStruct) {
      (OS << comment).indent(fieldIndent) << "/* empty struct */\n";
    } else {
      (OS << comment).indent(fieldIndent)
          << "[" << sizeOfStruct << " x i8] (type annotation not present)\n";
    }
  } else {
    for (unsigned i = 0; i < ST->getNumElements(); i++) {
      DxilFieldAnnotation &fieldAnnotation = annotation->GetFieldAnnotation(i);
      unsigned int fieldOffset;
      if (DL == nullptr) { // Constant buffer data layout
        fieldOffset = offset + fieldAnnotation.GetCBufferOffset();
      } else { // Normal data layout
        fieldOffset = offset + DL->getStructLayout(ST)->getElementOffset(i);
      }

      PrintFieldLayout(ST->getElementType(i), fieldAnnotation, typeSys, DL, OS,
                       comment, fieldOffset, fieldIndent, offsetIndent - 4);
    }
  }
  (OS << comment).indent(indent) << "\n";
  // The 2 in offsetIndent-indent-2 is for "} ".
  std::string varNameAndSemicolon = varName;
  varNameAndSemicolon += ';';
  (OS << comment).indent(indent)
      << "} " << left_justify(varNameAndSemicolon, offsetIndent - 2);
  OS << comment << " Offset:" << right_justify(std::to_string(offset), 5);
  if (sizeOfStruct)
    OS << " Size: " << right_justify(std::to_string(sizeOfStruct), 5);
  OS << "\n";

  OS << comment << "\n";
}

void PrintStructBufferDefinition(DxilResource *buf, DxilTypeSystem &typeSys,
                                 const DataLayout &DL, raw_string_ostream &OS,
                                 StringRef comment) {
  const unsigned offsetIndent = 50;

  OS << comment << " Resource bind info for " << buf->GetGlobalName() << "\n";
  OS << comment << " {\n";
  OS << comment << "\n";
  llvm::Type *RetTy = buf->GetRetType();
  // Skip none struct type.
  if (!RetTy->isStructTy() || HLMatrixType::isa(RetTy)) {
    llvm::Type *Ty = buf->GetHLSLType()->getPointerElementType();
    // For resource array, use element type.
    while (Ty->isArrayTy())
      Ty = Ty->getArrayElementType();
    // Get the struct buffer type like this %class.StructuredBuffer = type {
    // %struct.mat }.
    StructType *ST = cast<StructType>(Ty);
    DxilStructAnnotation *annotation = typeSys.GetStructAnnotation(ST);
    if (nullptr == annotation) {
      OS << comment << "   [" << DL.getTypeAllocSize(ST)
         << " x i8] (type annotation not present)\n";
    } else {
      DxilFieldAnnotation &fieldAnnotation = annotation->GetFieldAnnotation(0);
      fieldAnnotation.SetFieldName("$Element");
      PrintFieldLayout(
          RetTy, fieldAnnotation, typeSys, /*DL*/ nullptr, OS, comment,
          /*offset*/ 0, /*indent*/ 3, offsetIndent, DL.getTypeAllocSize(ST));
    }
    OS << comment << "\n";
  } else {
    StructType *ST = cast<StructType>(RetTy);

    DxilStructAnnotation *annotation = typeSys.GetStructAnnotation(ST);
    if (nullptr == annotation) {
      OS << comment << "   [" << DL.getTypeAllocSize(ST)
         << " x i8] (type annotation not present)\n";
    } else {
      PrintStructLayout(ST, typeSys, &DL, OS, comment, "$Element",
                        /*offset*/ 0, /*indent*/ 3, offsetIndent,
                        DL.getTypeAllocSize(ST));
    }
  }
  OS << comment << " }\n";
  OS << comment << "\n";
}

void PrintTBufferDefinition(DxilResource *buf, DxilTypeSystem &typeSys,
                            raw_string_ostream &OS, StringRef comment) {
  const unsigned offsetIndent = 50;
  llvm::Type *Ty = buf->GetHLSLType()->getPointerElementType();
  // For TextureBuffer<> buf[2], the array size is in Resource binding count
  // part.
  if (Ty->isArrayTy())
    Ty = Ty->getArrayElementType();

  DxilStructAnnotation *annotation =
      typeSys.GetStructAnnotation(cast<StructType>(Ty));
  OS << comment << " tbuffer " << buf->GetGlobalName() << "\n";
  OS << comment << " {\n";
  OS << comment << "\n";
  if (nullptr == annotation) {
    OS << comment << "   (type annotation not present)\n";
    OS << comment << "\n";
  } else {
    PrintStructLayout(cast<StructType>(Ty), typeSys, /*DL*/ nullptr, OS,
                      comment, buf->GetGlobalName(), /*offset*/ 0, /*indent*/ 3,
                      offsetIndent, annotation->GetCBufferSize());
  }
  OS << comment << " }\n";
  OS << comment << "\n";
}

void PrintCBufferDefinition(DxilCBuffer *buf, DxilTypeSystem &typeSys,
                            raw_string_ostream &OS, StringRef comment) {
  const unsigned offsetIndent = 50;
  llvm::Type *Ty = buf->GetHLSLType()->getPointerElementType();
  // For ConstantBuffer<> buf[2], the array size is in Resource binding count
  // part.
  if (Ty->isArrayTy())
    Ty = Ty->getArrayElementType();

  DxilStructAnnotation *annotation =
      typeSys.GetStructAnnotation(cast<StructType>(Ty));
  OS << comment << " cbuffer " << buf->GetGlobalName() << "\n";
  OS << comment << " {\n";
  OS << comment << "\n";
  if (nullptr == annotation) {
    OS << comment << "   [" << buf->GetSize()
       << " x i8] (type annotation not present)\n";
    OS << comment << "\n";
  } else {
    PrintStructLayout(cast<StructType>(Ty), typeSys, /*DL*/ nullptr, OS,
                      comment, buf->GetGlobalName(), /*offset*/ 0, /*indent*/ 3,
                      offsetIndent, buf->GetSize());
  }
  OS << comment << " }\n";
  OS << comment << "\n";
}

void PrintBufferDefinitions(DxilModule &M, raw_string_ostream &OS,
                            StringRef comment) {
  OS << comment << "\n"
     << comment << " Buffer Definitions:\n"
     << comment << "\n";
  DxilTypeSystem &typeSys = M.GetTypeSystem();

  for (auto &CBuf : M.GetCBuffers())
    PrintCBufferDefinition(CBuf.get(), typeSys, OS, comment);
  const DataLayout &layout = M.GetModule()->getDataLayout();
  for (auto &res : M.GetSRVs()) {
    if (res->IsStructuredBuffer())
      PrintStructBufferDefinition(res.get(), typeSys, layout, OS, comment);
    else if (res->IsTBuffer())
      PrintTBufferDefinition(res.get(), typeSys, OS, comment);
  }
  for (auto &res : M.GetUAVs()) {
    if (res->IsStructuredBuffer())
      PrintStructBufferDefinition(res.get(), typeSys, layout, OS, comment);
  }
}

#include "DxcDisassembler.inc"

LPCSTR ResourceKindToString(DXIL::ResourceKind RK) {
  switch (RK) {
  case DXIL::ResourceKind::Texture1D:
    return "Texture1D";
  case DXIL::ResourceKind::Texture2D:
    return "Texture2D";
  case DXIL::ResourceKind::Texture2DMS:
    return "Texture2DMS";
  case DXIL::ResourceKind::Texture3D:
    return "Texture3D";
  case DXIL::ResourceKind::TextureCube:
    return "TextureCube";
  case DXIL::ResourceKind::Texture1DArray:
    return "Texture1DArray";
  case DXIL::ResourceKind::Texture2DArray:
    return "Texture2DArray";
  case DXIL::ResourceKind::Texture2DMSArray:
    return "Texture2DMSArray";
  case DXIL::ResourceKind::TextureCubeArray:
    return "TextureCubeArray";
  case DXIL::ResourceKind::TypedBuffer:
    return "TypedBuffer";
  case DXIL::ResourceKind::RawBuffer:
    return "ByteAddressBuffer";
  case DXIL::ResourceKind::StructuredBuffer:
    return "StructuredBuffer";
  case DXIL::ResourceKind::CBuffer:
    return "CBuffer";
  case DXIL::ResourceKind::Sampler:
    return "Sampler";
  case DXIL::ResourceKind::TBuffer:
    return "TBuffer";
  case DXIL::ResourceKind::RTAccelerationStructure:
    return "RTAccelerationStructure";
  case DXIL::ResourceKind::FeedbackTexture2D:
    return "FeedbackTexture2D";
  case DXIL::ResourceKind::FeedbackTexture2DArray:
    return "FeedbackTexture2DArray";
  default:
    return "<invalid ResourceKind>";
  }
}

LPCSTR CompTypeToString(DXIL::ComponentType CompType) {
  switch (CompType) {
  case DXIL::ComponentType::I1:
    return "I1";
  case DXIL::ComponentType::I16:
    return "I16";
  case DXIL::ComponentType::U16:
    return "U16";
  case DXIL::ComponentType::I32:
    return "I32";
  case DXIL::ComponentType::U32:
    return "U32";
  case DXIL::ComponentType::I64:
    return "I64";
  case DXIL::ComponentType::U64:
    return "U64";
  case DXIL::ComponentType::F16:
    return "F16";
  case DXIL::ComponentType::F32:
    return "F32";
  case DXIL::ComponentType::F64:
    return "F64";
  case DXIL::ComponentType::SNormF16:
    return "SNormF16";
  case DXIL::ComponentType::UNormF16:
    return "UNormF16";
  case DXIL::ComponentType::SNormF32:
    return "SNormF32";
  case DXIL::ComponentType::UNormF32:
    return "UNormF32";
  case DXIL::ComponentType::SNormF64:
    return "SNormF64";
  case DXIL::ComponentType::UNormF64:
    return "UNormF64";
  default:
    return "<invalid CompType>";
  }
}

LPCSTR SamplerFeedbackTypeToString(DXIL::SamplerFeedbackType SFT) {
  switch (SFT) {
  case DXIL::SamplerFeedbackType::MinMip:
    return "MinMip";
  case DXIL::SamplerFeedbackType::MipRegionUsed:
    return "MipRegionUsed";
  default:
    return "<invalid sampler feedback type>";
  }
}

void PrintResourceProperties(DxilResourceProperties &RP,
                             formatted_raw_ostream &OS) {
  OS << "  resource: ";

  if (RP.getResourceClass() == DXIL::ResourceClass::CBuffer) {
    OS << "CBuffer";
    return;
  } else if (RP.getResourceClass() == DXIL::ResourceClass::SRV &&
             RP.getResourceKind() == DXIL::ResourceKind::TBuffer) {
    OS << "TBuffer";
    return;
  }

  if (RP.getResourceClass() == DXIL::ResourceClass::Sampler) {
    if (!RP.Basic.SamplerCmpOrHasCounter)
      OS << "SamplerState";
    else
      OS << "SamplerComparisonState";
    return;
  }

  bool bUAV = RP.isUAV();
  LPCSTR RW = bUAV ? (RP.Basic.IsROV ? "ROV" : "RW") : "";
  LPCSTR GC = bUAV && RP.Basic.IsGloballyCoherent ? "globallycoherent " : "";
  LPCSTR RC = bUAV && RP.Basic.IsReorderCoherent ? "reordercoherent " : "";
  LPCSTR COUNTER = bUAV && RP.Basic.SamplerCmpOrHasCounter ? ", counter" : "";

  switch (RP.getResourceKind()) {
  case DXIL::ResourceKind::Texture1D:
  case DXIL::ResourceKind::Texture2D:
  case DXIL::ResourceKind::Texture3D:
  case DXIL::ResourceKind::TextureCube:
  case DXIL::ResourceKind::Texture1DArray:
  case DXIL::ResourceKind::Texture2DArray:
  case DXIL::ResourceKind::TextureCubeArray:
  case DXIL::ResourceKind::TypedBuffer:
  case DXIL::ResourceKind::Texture2DMS:
  case DXIL::ResourceKind::Texture2DMSArray:
    OS << GC << RC << RW << ResourceKindToString(RP.getResourceKind());
    OS << "<";
    if (RP.Typed.CompCount > 1)
      OS << std::to_string(RP.Typed.CompCount) << "x";
    OS << CompTypeToString(RP.getCompType()) << ">";
    break;

  case DXIL::ResourceKind::RawBuffer:
    OS << GC << RC << RW << ResourceKindToString(RP.getResourceKind());
    break;

  case DXIL::ResourceKind::StructuredBuffer:
    OS << GC << RC << RW << ResourceKindToString(RP.getResourceKind());
    OS << "<stride=" << RP.StructStrideInBytes << COUNTER << ">";
    break;

  case DXIL::ResourceKind::RTAccelerationStructure:
    OS << ResourceKindToString(RP.getResourceKind());
    break;

  case DXIL::ResourceKind::FeedbackTexture2D:
  case DXIL::ResourceKind::FeedbackTexture2DArray:
    OS << ResourceKindToString(RP.getResourceKind());
    OS << "<" << SamplerFeedbackTypeToString(RP.SamplerFeedbackType) << ">";
    break;

  default:
    OS << "<invalid resource properties>";
    break;
  }
}

class DxcAssemblyAnnotationWriter : public llvm::AssemblyAnnotationWriter {
public:
  ~DxcAssemblyAnnotationWriter() {}
  void printInfoComment(const Value &V, formatted_raw_ostream &OS) override {
    if (const Instruction *I = dyn_cast<Instruction>(&V)) {
      if (isa<DbgInfoIntrinsic>(I)) {
        DILocalVariable *Var = nullptr;
        DIExpression *Expr = nullptr;
        if (const DbgDeclareInst *DI = dyn_cast<DbgDeclareInst>(I)) {
          Var = DI->getVariable();
          Expr = DI->getExpression();
        } else if (const DbgValueInst *DI = dyn_cast<DbgValueInst>(I)) {
          Var = DI->getVariable();
          Expr = DI->getExpression();
        }

        if (Var && Expr) {
          OS << " ; var:\"" << Var->getName() << "\""
             << " ";
          Expr->printAsBody(OS);
          if (DISubprogram *SubP = findFunctionScope(Var)) {
            OS << " func:\"" << SubP->getName() << "\"";
          }
        }
      } else {
        DebugLoc Loc = I->getDebugLoc();
        if (Loc && Loc.getLine() != 0)
          OS << " ; line:" << Loc.getLine() << " col:" << Loc.getCol();
      }
    }
    const CallInst *CI = dyn_cast<const CallInst>(&V);
    if (!CI) {
      return;
    }
    // TODO: annotate high-level operations where possible as well
    if (CI->getNumArgOperands() == 0 ||
        !CI->getCalledFunction()->getName().startswith("dx.op.")) {
      return;
    }
    const ConstantInt *CInt = dyn_cast<const ConstantInt>(CI->getArgOperand(0));
    if (!CInt) {
      // At this point, we know this is malformed; ignore.
      return;
    }

    unsigned opcodeVal = CInt->getZExtValue();
    if (opcodeVal >= (unsigned)DXIL::OpCode::NumOpCodes) {
      OS << "  ; invalid DXIL opcode #" << opcodeVal;
      return;
    }

    // TODO: if an argument references a resource, look it up and write the
    // name/binding
    DXIL::OpCode opcode = (DXIL::OpCode)opcodeVal;
    OS << "  ; " << hlsl::OP::GetOpCodeName(opcode)
       << OpCodeSignatures[opcodeVal];

    // Add extra decoding for certain ops
    switch (opcode) {
    case DXIL::OpCode::AnnotateHandle: {
      // Decode resource properties
      DxilInst_AnnotateHandle AH(const_cast<CallInst *>(CI));
      if (Constant *Props = dyn_cast<Constant>(AH.get_props())) {
        DxilResourceProperties RP =
            resource_helper::loadPropsFromConstant(*Props);
        PrintResourceProperties(RP, OS);
      }
    } break;
    default:
      break;
    }
  }

  static DISubprogram *findFunctionScope(DILocalVariable *var) {
    auto scope = var->getScope();
    if (scope) {
      return scope->getSubprogram();
    }
    return nullptr;
  }
};

void PrintPipelineStateValidationRuntimeInfo(const char *pBuffer,
                                             const uint32_t uBufferSize,
                                             DXIL::ShaderKind shaderKind,
                                             raw_string_ostream &OS,
                                             StringRef comment) {
  OS << comment << "\n"
     << comment << " Pipeline Runtime Information: \n"
     << comment << "\n";

  DxilPipelineStateValidation PSV;
  PSV.InitFromPSV0(pBuffer, uBufferSize);
  PSV.PrintPSVRuntimeInfo(OS, static_cast<uint8_t>(shaderKind), comment.data());

  OS << comment << "\n";
}
} // namespace

namespace dxcutil {

HRESULT Disassemble(IDxcBlob *pProgram, raw_string_ostream &Stream) {
  CComPtr<IDxcBlob> pPdbContainerBlob;
  {
    CComPtr<IStream> pStream;
    IFR(hlsl::CreateReadOnlyBlobStream(pProgram, &pStream));
    if (SUCCEEDED(hlsl::pdb::LoadDataFromStream(DxcGetThreadMallocNoRef(),
                                                pStream, &pPdbContainerBlob))) {
      pProgram = pPdbContainerBlob;
    }
  }

  const char *pIL = (const char *)pProgram->GetBufferPointer();
  uint32_t pILLength = pProgram->GetBufferSize();
  const char *pReflectionIL = nullptr;
  uint32_t pReflectionILLength = 0;
  const DxilPartHeader *pRDATPart = nullptr;
  if (const DxilContainerHeader *pContainer =
          IsDxilContainerLike(pIL, pILLength)) {
    if (!IsValidDxilContainer(pContainer, pILLength)) {
      return DXC_E_CONTAINER_INVALID;
    }

    DxilPartIterator it = std::find_if(begin(pContainer), end(pContainer),
                                       DxilPartIsType(DFCC_FeatureInfo));
    if (it != end(pContainer)) {
      PrintFeatureInfo(
          reinterpret_cast<const DxilShaderFeatureInfo *>(GetDxilPartData(*it)),
          Stream, /*comment*/ ";");
    }

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_InputSignature));
    if (it != end(pContainer)) {
      PrintSignature(
          "Input",
          reinterpret_cast<const DxilProgramSignature *>(GetDxilPartData(*it)),
          true, Stream, /*comment*/ ";");
    }
    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_OutputSignature));
    if (it != end(pContainer)) {
      PrintSignature(
          "Output",
          reinterpret_cast<const DxilProgramSignature *>(GetDxilPartData(*it)),
          false, Stream, /*comment*/ ";");
    }
    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_PatchConstantSignature));
    if (it != end(pContainer)) {
      PrintSignature(
          "Patch Constant",
          reinterpret_cast<const DxilProgramSignature *>(GetDxilPartData(*it)),
          false, Stream, /*comment*/ ";");
    }

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_ShaderDebugName));
    if (it != end(pContainer)) {
      const char *pDebugName;
      if (!GetDxilShaderDebugName(*it, &pDebugName, nullptr)) {
        Stream << "; shader debug name present; corruption detected\n";
      } else if (pDebugName && *pDebugName) {
        Stream << "; shader debug name: " << pDebugName << "\n";
      }
    }

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_ShaderHash));
    if (it != end(pContainer)) {
      const DxilShaderHash *pHashContent =
          reinterpret_cast<const DxilShaderHash *>(GetDxilPartData(*it));
      Stream << "; shader hash: ";
      for (int i = 0; i < 16; ++i)
        Stream << format("%.2x", pHashContent->Digest[i]);
      if (pHashContent->Flags & (uint32_t)DxilShaderHashFlags::IncludesSource)
        Stream << " (includes source)";
      Stream << "\n";
    }

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_DXIL));

    DxilPartIterator dbgit =
        std::find_if(begin(pContainer), end(pContainer),
                     DxilPartIsType(DFCC_ShaderDebugInfoDXIL));
    // Use dbg module if exist.
    if (dbgit != end(pContainer))
      it = dbgit;

    if (it == end(pContainer)) {
      return DXC_E_CONTAINER_MISSING_DXIL;
    }

    const DxilProgramHeader *pProgramHeader =
        reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(*it));
    if (!IsValidDxilProgramHeader(pProgramHeader, (*it)->PartSize)) {
      return DXC_E_CONTAINER_INVALID;
    }

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_PipelineStateValidation));
    if (it != end(pContainer)) {
      PrintPipelineStateValidationRuntimeInfo(
          GetDxilPartData(*it), (*it)->PartSize,
          GetVersionShaderType(pProgramHeader->ProgramVersion), Stream,
          /*comment*/ ";");
    }

    // RDAT
    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_RuntimeData));
    if (it != end(pContainer)) {
      pRDATPart = *it;
    }

    GetDxilProgramBitcode(pProgramHeader, &pIL, &pILLength);

    it = std::find_if(begin(pContainer), end(pContainer),
                      DxilPartIsType(DFCC_ShaderStatistics));
    if (it != end(pContainer)) {
      // If this part exists, use it for reflection data, probably stripped from
      // DXIL part.
      const DxilProgramHeader *pReflectionProgramHeader =
          reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(*it));
      if (IsValidDxilProgramHeader(pReflectionProgramHeader, (*it)->PartSize)) {
        GetDxilProgramBitcode(pReflectionProgramHeader, &pReflectionIL,
                              &pReflectionILLength);
      }
    }

  } else {
    const DxilProgramHeader *pProgramHeader =
        reinterpret_cast<const DxilProgramHeader *>(pIL);
    if (IsValidDxilProgramHeader(pProgramHeader, pILLength)) {
      GetDxilProgramBitcode(pProgramHeader, &pIL, &pILLength);
    }
  }

  std::string DiagStr;
  llvm::LLVMContext llvmContext;
  std::unique_ptr<llvm::Module> pModule(dxilutil::LoadModuleFromBitcode(
      llvm::StringRef(pIL, pILLength), llvmContext, DiagStr));
  if (pModule.get() == nullptr) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  std::unique_ptr<llvm::Module> pReflectionModule;
  if (pReflectionIL && pReflectionILLength) {
    pReflectionModule = dxilutil::LoadModuleFromBitcode(
        llvm::StringRef(pReflectionIL, pReflectionILLength), llvmContext,
        DiagStr);
    if (pReflectionModule.get() == nullptr) {
      return DXC_E_IR_VERIFICATION_FAILED;
    }
  }

  if (pModule->getNamedMetadata("dx.version")) {
    DxilModule &dxilModule = pModule->GetOrCreateDxilModule();
    DxilModule &dxilReflectionModule =
        pReflectionModule.get() ? pReflectionModule->GetOrCreateDxilModule()
                                : dxilModule;

    if (!dxilModule.GetShaderModel()->IsLib()) {
      PrintDxilSignature("Input", dxilModule.GetInputSignature(), Stream,
                         /*comment*/ ";");
      if (dxilModule.GetShaderModel()->IsMS()) {
        PrintDxilSignature("Vertex Output", dxilModule.GetOutputSignature(),
                           Stream,
                           /*comment*/ ";");
        PrintDxilSignature("Primitive Output",
                           dxilModule.GetPatchConstOrPrimSignature(), Stream,
                           /*comment*/ ";");
      } else {
        PrintDxilSignature("Output", dxilModule.GetOutputSignature(), Stream,
                           /*comment*/ ";");
        PrintDxilSignature("Patch Constant",
                           dxilModule.GetPatchConstOrPrimSignature(), Stream,
                           /*comment*/ ";");
      }
    }
    PrintBufferDefinitions(dxilReflectionModule, Stream, /*comment*/ ";");
    PrintResourceBindings(dxilReflectionModule, Stream, /*comment*/ ";");
    PrintViewIdState(dxilReflectionModule, Stream, /*comment*/ ";");

    if (pRDATPart) {
      RDAT::DxilRuntimeData runtimeData(GetDxilPartData(pRDATPart),
                                        pRDATPart->PartSize);
      // TODO: Print the rest of the RDAT info
      if (runtimeData.GetSubobjectTable()) {
        dxilModule.ResetSubobjects(new DxilSubobjects());
        if (!LoadSubobjectsFromRDAT(*dxilModule.GetSubobjects(), runtimeData)) {
          Stream << "; error occurred while loading Subobjects from RDAT.\n";
        }
      }
    }
    if (dxilModule.GetSubobjects()) {
      PrintSubobjects(*dxilModule.GetSubobjects(), Stream, /*comment*/ ";");
    }
  }
  DxcAssemblyAnnotationWriter w;
  pModule->print(Stream, &w);
  // if (pReflectionModule) {
  //   Stream << "\n========== Reflection Module from STAT part ==========\n";
  //   pReflectionModule->print(Stream, &w);
  // }
  Stream.flush();
  return S_OK;
}
} // namespace dxcutil
