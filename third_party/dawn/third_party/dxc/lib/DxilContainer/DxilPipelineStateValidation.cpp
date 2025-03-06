///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPipelineStateValidation.cpp                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utils for PSV.                                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilResource.h"
#include "dxc/DXIL/DxilResourceBase.h"
#include "dxc/DXIL/DxilSignature.h"
#include "dxc/DXIL/DxilSignatureElement.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace hlsl;
using namespace llvm;

uint32_t hlsl::GetPSVVersion(uint32_t ValMajor, uint32_t ValMinor) {
  unsigned PSVVersion = MAX_PSV_VERSION;
  // Constraint PSVVersion based on validator version
  if (DXIL::CompareVersions(ValMajor, ValMinor, 1, 1) < 0)
    PSVVersion = 0;
  else if (DXIL::CompareVersions(ValMajor, ValMinor, 1, 6) < 0)
    PSVVersion = 1;
  else if (DXIL::CompareVersions(ValMajor, ValMinor, 1, 8) < 0)
    PSVVersion = 2;
  return PSVVersion;
}

void hlsl::InitPSVResourceBinding(PSVResourceBindInfo0 *Bind0,
                                  PSVResourceBindInfo1 *Bind1,
                                  DxilResourceBase *Res) {
  Bind0->Space = Res->GetSpaceID();
  Bind0->LowerBound = Res->GetLowerBound();
  Bind0->UpperBound = Res->GetUpperBound();

  PSVResourceType ResType = PSVResourceType::Invalid;
  bool IsUAV = Res->GetClass() == DXIL::ResourceClass::UAV;
  switch (Res->GetKind()) {
  case DXIL::ResourceKind::Sampler:
    ResType = PSVResourceType::Sampler;
    break;
  case DXIL::ResourceKind::CBuffer:
    ResType = PSVResourceType::CBV;
    break;
  case DXIL::ResourceKind::StructuredBuffer:
    ResType =
        IsUAV ? PSVResourceType::UAVStructured : PSVResourceType::SRVStructured;
    if (IsUAV) {
      DxilResource *UAV = static_cast<DxilResource *>(Res);
      if (UAV->HasCounter())
        ResType = PSVResourceType::UAVStructuredWithCounter;
    }
    break;
  case DXIL::ResourceKind::RTAccelerationStructure:
    ResType = PSVResourceType::SRVRaw;
    break;
  case DXIL::ResourceKind::RawBuffer:
    ResType = IsUAV ? PSVResourceType::UAVRaw : PSVResourceType::SRVRaw;
    break;
  default:
    ResType = IsUAV ? PSVResourceType::UAVTyped : PSVResourceType::SRVTyped;
    break;
  }

  Bind0->ResType = static_cast<uint32_t>(ResType);
  if (Bind1) {
    Bind1->ResKind = static_cast<uint32_t>(Res->GetKind());
    Bind1->ResFlags = 0;
    if (IsUAV) {
      DxilResource *UAV = static_cast<DxilResource *>(Res);
      unsigned ResFlags =
          UAV->HasAtomic64Use()
              ? static_cast<unsigned>(PSVResourceFlag::UsedByAtomic64)
              : 0;
      Bind1->ResFlags = ResFlags;
    }
  }
}

void hlsl::InitPSVSignatureElement(PSVSignatureElement0 &E,
                                   const DxilSignatureElement &SE,
                                   bool i1ToUnknownCompat) {
  memset(&E, 0, sizeof(PSVSignatureElement0));
  DXASSERT_NOMSG(SE.GetRows() <= 32);
  E.Rows = (uint8_t)SE.GetRows();
  DXASSERT_NOMSG(SE.GetCols() <= 4);
  E.ColsAndStart = (uint8_t)SE.GetCols() & 0xF;
  if (SE.IsAllocated()) {
    DXASSERT_NOMSG(SE.GetStartCol() < 4);
    DXASSERT_NOMSG(SE.GetStartRow() < 32);
    E.ColsAndStart |= 0x40 | (SE.GetStartCol() << 4);
    E.StartRow = (uint8_t)SE.GetStartRow();
  }
  E.SemanticKind = (uint8_t)SE.GetKind();
  E.ComponentType = (uint8_t)CompTypeToSigCompType(SE.GetCompType().GetKind(),
                                                   i1ToUnknownCompat);
  E.InterpolationMode = (uint8_t)SE.GetInterpolationMode()->GetKind();
  DXASSERT_NOMSG(SE.GetOutputStream() < 4);
  E.DynamicMaskAndStream = (uint8_t)((SE.GetOutputStream() & 0x3) << 4);
  E.DynamicMaskAndStream |= (SE.GetDynIdxCompMask()) & 0xF;
}

void hlsl::SetupPSVInitInfo(PSVInitInfo &InitInfo, const DxilModule &DM) {
  // Constraint PSVVersion based on validator version
  unsigned ValMajor, ValMinor;
  DM.GetValidatorVersion(ValMajor, ValMinor);
  unsigned PSVVersionConstraint = hlsl::GetPSVVersion(ValMajor, ValMinor);
  if (InitInfo.PSVVersion > PSVVersionConstraint)
    InitInfo.PSVVersion = PSVVersionConstraint;

  const ShaderModel *SM = DM.GetShaderModel();
  uint32_t uCBuffers = DM.GetCBuffers().size();
  uint32_t uSamplers = DM.GetSamplers().size();
  uint32_t uSRVs = DM.GetSRVs().size();
  uint32_t uUAVs = DM.GetUAVs().size();
  InitInfo.ResourceCount = uCBuffers + uSamplers + uSRVs + uUAVs;

  if (InitInfo.PSVVersion > 0) {
    InitInfo.ShaderStage = (PSVShaderKind)SM->GetKind();
    InitInfo.SigInputElements = DM.GetInputSignature().GetElements().size();
    InitInfo.SigPatchConstOrPrimElements =
        DM.GetPatchConstOrPrimSignature().GetElements().size();
    InitInfo.SigOutputElements = DM.GetOutputSignature().GetElements().size();

    // Set up ViewID and signature dependency info
    InitInfo.UsesViewID = DM.m_ShaderFlags.GetViewID() ? true : false;
    InitInfo.SigInputVectors = DM.GetInputSignature().NumVectorsUsed(0);
    for (unsigned streamIndex = 0; streamIndex < 4; streamIndex++) {
      InitInfo.SigOutputVectors[streamIndex] =
          DM.GetOutputSignature().NumVectorsUsed(streamIndex);
    }
    InitInfo.SigPatchConstOrPrimVectors = 0;
    if (SM->IsHS() || SM->IsDS() || SM->IsMS()) {
      InitInfo.SigPatchConstOrPrimVectors =
          DM.GetPatchConstOrPrimSignature().NumVectorsUsed(0);
    }
  }
}

void hlsl::SetShaderProps(PSVRuntimeInfo0 *pInfo, const DxilModule &DM) {
  const ShaderModel *SM = DM.GetShaderModel();
  pInfo->MinimumExpectedWaveLaneCount = 0;
  pInfo->MaximumExpectedWaveLaneCount = (uint32_t)-1;

  switch (SM->GetKind()) {
  case ShaderModel::Kind::Vertex: {
    pInfo->VS.OutputPositionPresent = 0;
    const DxilSignature &S = DM.GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->VS.OutputPositionPresent = 1;
        break;
      }
    }
    break;
  }
  case ShaderModel::Kind::Hull: {
    pInfo->HS.InputControlPointCount = (uint32_t)DM.GetInputControlPointCount();
    pInfo->HS.OutputControlPointCount =
        (uint32_t)DM.GetOutputControlPointCount();
    pInfo->HS.TessellatorDomain = (uint32_t)DM.GetTessellatorDomain();
    pInfo->HS.TessellatorOutputPrimitive =
        (uint32_t)DM.GetTessellatorOutputPrimitive();
    break;
  }
  case ShaderModel::Kind::Domain: {
    pInfo->DS.InputControlPointCount = (uint32_t)DM.GetInputControlPointCount();
    pInfo->DS.OutputPositionPresent = 0;
    const DxilSignature &S = DM.GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->DS.OutputPositionPresent = 1;
        break;
      }
    }
    pInfo->DS.TessellatorDomain = (uint32_t)DM.GetTessellatorDomain();
    break;
  }
  case ShaderModel::Kind::Geometry: {
    pInfo->GS.InputPrimitive = (uint32_t)DM.GetInputPrimitive();
    // NOTE: For OutputTopology, pick one from a used stream, or if none
    // are used, use stream 0, and set OutputStreamMask to 1.
    pInfo->GS.OutputTopology = (uint32_t)DM.GetStreamPrimitiveTopology();
    pInfo->GS.OutputStreamMask = DM.GetActiveStreamMask();
    if (pInfo->GS.OutputStreamMask == 0) {
      pInfo->GS.OutputStreamMask = 1; // This is what runtime expects.
    }
    pInfo->GS.OutputPositionPresent = 0;
    const DxilSignature &S = DM.GetOutputSignature();
    for (auto &&E : S.GetElements()) {
      if (E->GetKind() == Semantic::Kind::Position) {
        // Ideally, we might check never writes mask here,
        // but this is not yet part of the signature element in Dxil
        pInfo->GS.OutputPositionPresent = 1;
        break;
      }
    }
    break;
  }
  case ShaderModel::Kind::Pixel: {
    pInfo->PS.DepthOutput = 0;
    pInfo->PS.SampleFrequency = 0;
    {
      const DxilSignature &S = DM.GetInputSignature();
      for (auto &&E : S.GetElements()) {
        if (E->GetInterpolationMode()->IsAnySample() ||
            E->GetKind() == Semantic::Kind::SampleIndex) {
          pInfo->PS.SampleFrequency = 1;
        }
      }
    }
    {
      const DxilSignature &S = DM.GetOutputSignature();
      for (auto &&E : S.GetElements()) {
        if (E->IsAnyDepth()) {
          pInfo->PS.DepthOutput = 1;
          break;
        }
      }
    }
    break;
  }
  case ShaderModel::Kind::Compute: {
    DxilWaveSize waveSize = DM.GetWaveSize();
    pInfo->MinimumExpectedWaveLaneCount = 0;
    pInfo->MaximumExpectedWaveLaneCount = UINT32_MAX;
    if (waveSize.IsDefined()) {
      pInfo->MinimumExpectedWaveLaneCount = waveSize.Min;
      pInfo->MaximumExpectedWaveLaneCount =
          waveSize.IsRange() ? waveSize.Max : waveSize.Min;
    }
    break;
  }
  case ShaderModel::Kind::Library:
  case ShaderModel::Kind::Invalid:
    // Library and Invalid not relevant to PSVRuntimeInfo0
    break;
  case ShaderModel::Kind::Mesh: {
    pInfo->MS.MaxOutputVertices = (uint16_t)DM.GetMaxOutputVertices();
    pInfo->MS.MaxOutputPrimitives = (uint16_t)DM.GetMaxOutputPrimitives();
    Module *mod = DM.GetModule();
    const DataLayout &DL = mod->getDataLayout();
    unsigned totalByteSize = 0;
    for (GlobalVariable &GV : mod->globals()) {
      PointerType *gvPtrType = cast<PointerType>(GV.getType());
      if (gvPtrType->getAddressSpace() == hlsl::DXIL::kTGSMAddrSpace) {
        Type *gvType = gvPtrType->getPointerElementType();
        unsigned byteSize = DL.getTypeAllocSize(gvType);
        totalByteSize += byteSize;
      }
    }
    pInfo->MS.GroupSharedBytesUsed = totalByteSize;
    pInfo->MS.PayloadSizeInBytes = DM.GetPayloadSizeInBytes();
    break;
  }
  case ShaderModel::Kind::Amplification: {
    pInfo->AS.PayloadSizeInBytes = DM.GetPayloadSizeInBytes();
    break;
  }
  }
}

void hlsl::SetShaderProps(PSVRuntimeInfo1 *pInfo1, const DxilModule &DM) {
  assert(pInfo1);
  const ShaderModel *SM = DM.GetShaderModel();
  switch (SM->GetKind()) {
  case ShaderModel::Kind::Geometry:
    pInfo1->MaxVertexCount = (uint16_t)DM.GetMaxVertexCount();
    break;
  case ShaderModel::Kind::Mesh:
    pInfo1->MS1.MeshOutputTopology = (uint8_t)DM.GetMeshOutputTopology();
    break;
  default:
    break;
  }
}

void hlsl::SetShaderProps(PSVRuntimeInfo2 *pInfo2, const DxilModule &DM) {
  assert(pInfo2);
  const ShaderModel *SM = DM.GetShaderModel();
  switch (SM->GetKind()) {
  case ShaderModel::Kind::Compute:
  case ShaderModel::Kind::Mesh:
  case ShaderModel::Kind::Amplification:
    pInfo2->NumThreadsX = DM.GetNumThreads(0);
    pInfo2->NumThreadsY = DM.GetNumThreads(1);
    pInfo2->NumThreadsZ = DM.GetNumThreads(2);
    break;
  default:
    break;
  }
}

void PSVResourceBindInfo0::Print(raw_ostream &OS) const {
  OS << "PSVResourceBindInfo:\n";
  OS << "  Space: " << Space << "\n";
  OS << "  LowerBound: " << LowerBound << "\n";
  OS << "  UpperBound: " << UpperBound << "\n";
  switch (static_cast<PSVResourceType>(ResType)) {
  case PSVResourceType::CBV:
    OS << "  ResType: CBV\n";
    break;
  case PSVResourceType::Sampler:
    OS << "  ResType: Sampler\n";
    break;
  case PSVResourceType::SRVRaw:
    OS << "  ResType: SRVRaw\n";
    break;
  case PSVResourceType::SRVStructured:
    OS << "  ResType: SRVStructured\n";
    break;
  case PSVResourceType::SRVTyped:
    OS << "  ResType: SRVTyped\n";
    break;
  case PSVResourceType::UAVRaw:
    OS << "  ResType: UAVRaw\n";
    break;
  case PSVResourceType::UAVStructured:
    OS << "  ResType: UAVStructured\n";
    break;
  case PSVResourceType::UAVTyped:
    OS << "  ResType: UAVTyped\n";
    break;
  case PSVResourceType::UAVStructuredWithCounter:
    OS << "  ResType: UAVStructuredWithCounter\n";
    break;
  case PSVResourceType::Invalid:
    OS << "  ResType: Invalid\n";
    break;
  default:
    OS << "  ResType: Unknown\n";
    break;
  }
}

void PSVResourceBindInfo1::Print(raw_ostream &OS) const {
  PSVResourceBindInfo0::Print(OS);
  switch (static_cast<PSVResourceKind>(ResKind)) {
  case PSVResourceKind::Invalid:
    OS << "  ResKind: Invalid\n";
    break;
  case PSVResourceKind::CBuffer:
    OS << "  ResKind: CBuffer\n";
    break;
  case PSVResourceKind::Sampler:
    OS << "  ResKind: Sampler\n";
    break;
  case PSVResourceKind::FeedbackTexture2D:
    OS << "  ResKind: FeedbackTexture2D\n";
    break;
  case PSVResourceKind::FeedbackTexture2DArray:
    OS << "  ResKind: FeedbackTexture2DArray\n";
    break;
  case PSVResourceKind::RawBuffer:
    OS << "  ResKind: RawBuffer\n";
    break;
  case PSVResourceKind::StructuredBuffer:
    OS << "  ResKind: StructuredBuffer\n";
    break;
  case PSVResourceKind::TypedBuffer:
    OS << "  ResKind: TypedBuffer\n";
    break;
  case PSVResourceKind::RTAccelerationStructure:
    OS << "  ResKind: RTAccelerationStructure\n";
    break;
  case PSVResourceKind::TBuffer:
    OS << "  ResKind: TBuffer\n";
    break;
  case PSVResourceKind::Texture1D:
    OS << "  ResKind: Texture1D\n";
    break;
  case PSVResourceKind::Texture1DArray:
    OS << "  ResKind: Texture1DArray\n";
    break;
  case PSVResourceKind::Texture2D:
    OS << "  ResKind: Texture2D\n";
    break;
  case PSVResourceKind::Texture2DArray:
    OS << "  ResKind: Texture2DArray\n";
    break;
  case PSVResourceKind::Texture2DMS:
    OS << "  ResKind: Texture2DMS\n";
    break;
  case PSVResourceKind::Texture2DMSArray:
    OS << "  ResKind: Texture2DMSArray\n";
    break;
  case PSVResourceKind::Texture3D:
    OS << "  ResKind: Texture3D\n";
    break;
  case PSVResourceKind::TextureCube:
    OS << "  ResKind: TextureCube\n";
    break;
  case PSVResourceKind::TextureCubeArray:
    OS << "  ResKind: TextureCubeArray\n";
    break;
  }
  if (ResFlags == 0) {
    OS << "  ResFlags: None\n";
  } else {
    OS << "  ResFlags: ";
    if (ResFlags & static_cast<uint32_t>(PSVResourceFlag::UsedByAtomic64)) {
      OS << "UsedByAtomic64 ";
    }
    OS << "\n";
  }
}

void PSVSignatureElement::Print(raw_ostream &OS) const {
  Print(OS, GetSemanticName(), GetSemanticIndexes());
}

void PSVSignatureElement::Print(raw_ostream &OS, const char *Name,
                                const uint32_t *SemanticIndexes) const {
  OS << "PSVSignatureElement:\n";
  OS << "  SemanticName: " << Name << "\n";
  OS << "  SemanticIndex: ";
  for (unsigned i = 0; i < GetRows(); ++i) {
    OS << *(SemanticIndexes + i) << " ";
  }
  OS << "\n";
  OS << "  IsAllocated: " << IsAllocated() << "\n";
  OS << "  StartRow: " << GetStartRow() << "\n";
  OS << "  StartCol: " << GetStartCol() << "\n";
  OS << "  Rows: " << GetRows() << "\n";
  OS << "  Cols: " << GetCols() << "\n";
  OS << "  SemanticKind: ";
  switch (GetSemanticKind()) {
  case PSVSemanticKind::Arbitrary:
    OS << "Arbitrary\n";
    break;
  case PSVSemanticKind::VertexID:
    OS << "VertexID\n";
    break;
  case PSVSemanticKind::InstanceID:
    OS << "InstanceID\n";
    break;
  case PSVSemanticKind::Position:
    OS << "Position\n";
    break;
  case PSVSemanticKind::RenderTargetArrayIndex:
    OS << "RenderTargetArrayIndex\n";
    break;
  case PSVSemanticKind::ViewPortArrayIndex:
    OS << "ViewPortArrayIndex\n";
    break;
  case PSVSemanticKind::ClipDistance:
    OS << "ClipDistance\n";
    break;
  case PSVSemanticKind::CullDistance:
    OS << "CullDistance\n";
    break;
  case PSVSemanticKind::OutputControlPointID:
    OS << "OutputControlPointID\n";
    break;
  case PSVSemanticKind::DomainLocation:
    OS << "DomainLocation\n";
    break;
  case PSVSemanticKind::PrimitiveID:
    OS << "PrimitiveID\n";
    break;
  case PSVSemanticKind::GSInstanceID:
    OS << "GSInstanceID\n";
    break;
  case PSVSemanticKind::SampleIndex:
    OS << "SampleIndex\n";
    break;
  case PSVSemanticKind::IsFrontFace:
    OS << "IsFrontFace\n";
    break;
  case PSVSemanticKind::Coverage:
    OS << "Coverage\n";
    break;
  case PSVSemanticKind::InnerCoverage:
    OS << "InnerCoverage\n";
    break;
  case PSVSemanticKind::Target:
    OS << "Target\n";
    break;
  case PSVSemanticKind::Depth:
    OS << "Depth\n";
    break;
  case PSVSemanticKind::DepthLessEqual:
    OS << "DepthLessEqual\n";
    break;
  case PSVSemanticKind::DepthGreaterEqual:
    OS << "DepthGreaterEqual\n";
    break;
  case PSVSemanticKind::StencilRef:
    OS << "StencilRef\n";
    break;
  case PSVSemanticKind::DispatchThreadID:
    OS << "DispatchThreadID\n";
    break;
  case PSVSemanticKind::GroupID:
    OS << "GroupID\n";
    break;
  case PSVSemanticKind::GroupIndex:
    OS << "GroupIndex\n";
    break;
  case PSVSemanticKind::GroupThreadID:
    OS << "GroupThreadID\n";
    break;
  case PSVSemanticKind::TessFactor:
    OS << "TessFactor\n";
    break;
  case PSVSemanticKind::InsideTessFactor:
    OS << "InsideTessFactor\n";
    break;
  case PSVSemanticKind::ViewID:
    OS << "ViewID\n";
    break;
  case PSVSemanticKind::Barycentrics:
    OS << "Barycentrics\n";
    break;
  case PSVSemanticKind::ShadingRate:
    OS << "ShadingRate\n";
    break;
  case PSVSemanticKind::CullPrimitive:
    OS << "CullPrimitive\n";
    break;
  case PSVSemanticKind::Invalid:
    OS << "Invalid\n";
    break;
  }

  OS << "  InterpolationMode: " << GetInterpolationMode() << "\n";
  OS << "  OutputStream: " << GetOutputStream() << "\n";
  OS << "  ComponentType: " << GetComponentType() << "\n";
  OS << "  DynamicIndexMask: " << GetDynamicIndexMask() << "\n";
}

void PSVComponentMask::Print(raw_ostream &OS, const char *InputSetName,
                             const char *OutputSetName) const {
  OS << "  " << InputSetName << " influencing " << OutputSetName << " :";
  bool Empty = true;
  for (unsigned i = 0; i < NumVectors; ++i) {
    for (unsigned j = 0; j < 32; ++j) {
      uint32_t Index = i * 32 + j;
      if (Get(Index)) {
        OS << " " << Index << " ";
        Empty = false;
      }
    }
  }
  if (Empty)
    OS << "  None";
  OS << "\n";
}

void PSVDependencyTable::Print(raw_ostream &OS, const char *InputSetName,
                               const char *OutputSetName) const {
  OS << InputSetName << " contributing to computation of " << OutputSetName
     << ":";
  if (!IsValid()) {
    OS << "  None\n";
    return;
  }
  OS << "\n";
  for (unsigned i = 0; i < InputVectors; ++i) {
    for (unsigned j = 0; j < 4; ++j) {
      unsigned Index = i * 4 + j;
      const PSVComponentMask Mask = GetMaskForInput(Index);

      std::string InputName = InputSetName;
      InputName += "[" + std::to_string(Index) + "]";
      Mask.Print(OS, InputName.c_str(), OutputSetName);
    }
  }
}

void hlsl::PrintPSVRuntimeInfo(llvm::raw_ostream &OS, PSVRuntimeInfo0 *pInfo0,
                               PSVRuntimeInfo1 *pInfo1, PSVRuntimeInfo2 *pInfo2,
                               PSVRuntimeInfo3 *pInfo3, uint8_t ShaderKind,
                               const char *EntryName, const char *Comment) {
  if (pInfo1 && pInfo1->ShaderStage != ShaderKind)
    ShaderKind = pInfo1->ShaderStage;
  OS << Comment << "PSVRuntimeInfo:\n";
  switch (static_cast<PSVShaderKind>(ShaderKind)) {
  case PSVShaderKind::Hull: {
    OS << Comment << " Hull Shader\n";
    OS << Comment
       << " InputControlPointCount=" << pInfo0->HS.InputControlPointCount
       << "\n";
    OS << Comment
       << " OutputControlPointCount=" << pInfo0->HS.OutputControlPointCount
       << "\n";
    OS << Comment << " Domain=";
    DXIL::TessellatorDomain domain =
        static_cast<DXIL::TessellatorDomain>(pInfo0->HS.TessellatorDomain);
    switch (domain) {
    case DXIL::TessellatorDomain::IsoLine:
      OS << "isoline\n";
      break;
    case DXIL::TessellatorDomain::Tri:
      OS << "tri\n";
      break;
    case DXIL::TessellatorDomain::Quad:
      OS << "quad\n";
      break;
    default:
      OS << "invalid\n";
      break;
    }
    OS << Comment << " OutputPrimitive=";
    DXIL::TessellatorOutputPrimitive primitive =
        static_cast<DXIL::TessellatorOutputPrimitive>(
            pInfo0->HS.TessellatorOutputPrimitive);
    switch (primitive) {
    case DXIL::TessellatorOutputPrimitive::Point:
      OS << "point\n";
      break;
    case DXIL::TessellatorOutputPrimitive::Line:
      OS << "line\n";
      break;
    case DXIL::TessellatorOutputPrimitive::TriangleCW:
      OS << "triangle_cw\n";
      break;
    case DXIL::TessellatorOutputPrimitive::TriangleCCW:
      OS << "triangle_ccw\n";
      break;
    default:
      OS << "invalid\n";
      break;
    }
  } break;
  case PSVShaderKind::Domain:
    OS << Comment << " Domain Shader\n";
    OS << Comment
       << " InputControlPointCount=" << pInfo0->DS.InputControlPointCount
       << "\n";
    OS << Comment
       << " OutputPositionPresent=" << (bool)pInfo0->DS.OutputPositionPresent
       << "\n";
    break;
  case PSVShaderKind::Geometry: {
    OS << Comment << " Geometry Shader\n";
    OS << Comment << " InputPrimitive=";
    DXIL::InputPrimitive primitive =
        static_cast<DXIL::InputPrimitive>(pInfo0->GS.InputPrimitive);
    switch (primitive) {
    case DXIL::InputPrimitive::Point:
      OS << "point\n";
      break;
    case DXIL::InputPrimitive::Line:
      OS << "line\n";
      break;
    case DXIL::InputPrimitive::LineWithAdjacency:
      OS << "lineadj\n";
      break;
    case DXIL::InputPrimitive::Triangle:
      OS << "triangle\n";
      break;
    case DXIL::InputPrimitive::TriangleWithAdjacency:
      OS << "triangleadj\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch1:
      OS << "patch1\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch2:
      OS << "patch2\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch3:
      OS << "patch3\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch4:
      OS << "patch4\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch5:
      OS << "patch5\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch6:
      OS << "patch6\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch7:
      OS << "patch7\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch8:
      OS << "patch8\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch9:
      OS << "patch9\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch10:
      OS << "patch10\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch11:
      OS << "patch11\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch12:
      OS << "patch12\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch13:
      OS << "patch13\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch14:
      OS << "patch14\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch15:
      OS << "patch15\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch16:
      OS << "patch16\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch17:
      OS << "patch17\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch18:
      OS << "patch18\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch19:
      OS << "patch19\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch20:
      OS << "patch20\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch21:
      OS << "patch21\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch22:
      OS << "patch22\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch23:
      OS << "patch23\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch24:
      OS << "patch24\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch25:
      OS << "patch25\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch26:
      OS << "patch26\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch27:
      OS << "patch27\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch28:
      OS << "patch28\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch29:
      OS << "patch29\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch30:
      OS << "patch30\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch31:
      OS << "patch31\n";
      break;
    case DXIL::InputPrimitive::ControlPointPatch32:
      OS << "patch32\n";
      break;
    default:
      OS << "invalid\n";
      break;
    }
    OS << Comment << " OutputTopology=";
    DXIL::PrimitiveTopology topology =
        static_cast<DXIL::PrimitiveTopology>(pInfo0->GS.OutputTopology);
    switch (topology) {
    case DXIL::PrimitiveTopology::PointList:
      OS << "point\n";
      break;
    case DXIL::PrimitiveTopology::LineStrip:
      OS << "line\n";
      break;
    case DXIL::PrimitiveTopology::TriangleStrip:
      OS << "triangle\n";
      break;
    default:
      OS << "invalid\n";
      break;
    }
    OS << Comment << " OutputStreamMask=" << pInfo0->GS.OutputStreamMask
       << "\n";
    OS << Comment
       << " OutputPositionPresent=" << (bool)pInfo0->GS.OutputPositionPresent
       << "\n";
  } break;
  case PSVShaderKind::Vertex:
    OS << Comment << " Vertex Shader\n";
    OS << Comment
       << " OutputPositionPresent=" << (bool)pInfo0->VS.OutputPositionPresent
       << "\n";
    break;
  case PSVShaderKind::Pixel:
    OS << Comment << " Pixel Shader\n";
    OS << Comment << " DepthOutput=" << (bool)pInfo0->PS.DepthOutput << "\n";
    OS << Comment << " SampleFrequency=" << (bool)pInfo0->PS.SampleFrequency
       << "\n";
    break;
  case PSVShaderKind::Compute:
    OS << Comment << " Compute Shader\n";
    if (pInfo2) {
      OS << Comment << " NumThreads=(" << pInfo2->NumThreadsX << ","
         << pInfo2->NumThreadsY << "," << pInfo2->NumThreadsZ << ")\n";
    }
    break;
  case PSVShaderKind::Amplification:
    OS << Comment << " Amplification Shader\n";
    if (pInfo2) {
      OS << Comment << " NumThreads=(" << pInfo2->NumThreadsX << ","
         << pInfo2->NumThreadsY << "," << pInfo2->NumThreadsZ << ")\n";
    }
    break;
  case PSVShaderKind::Mesh:
    OS << Comment << " Mesh Shader\n";
    if (pInfo1) {
      OS << Comment << " MeshOutputTopology=";
      DXIL::MeshOutputTopology topology =
          static_cast<DXIL::MeshOutputTopology>(pInfo1->MS1.MeshOutputTopology);
      switch (topology) {
      case DXIL::MeshOutputTopology::Undefined:
        OS << "undefined\n";
        break;
      case DXIL::MeshOutputTopology::Line:
        OS << "line\n";
        break;
      case DXIL::MeshOutputTopology::Triangle:
        OS << "triangle\n";
        break;
      default:
        OS << "invalid\n";
        break;
      }
    }
    if (pInfo2) {
      OS << Comment << " NumThreads=(" << pInfo2->NumThreadsX << ","
         << pInfo2->NumThreadsY << "," << pInfo2->NumThreadsZ << ")\n";
    }
    break;
  case PSVShaderKind::Library:
  case PSVShaderKind::Invalid:
    // Nothing to print for these shader kinds.
    break;
  }

  if (pInfo0->MinimumExpectedWaveLaneCount ==
      pInfo0->MaximumExpectedWaveLaneCount) {
    OS << Comment << " WaveSize=" << pInfo0->MinimumExpectedWaveLaneCount
       << "\n";
  } else {

    OS << Comment << " MinimumExpectedWaveLaneCount: "
       << pInfo0->MinimumExpectedWaveLaneCount << "\n";
    OS << Comment << " MaximumExpectedWaveLaneCount: "
       << pInfo0->MaximumExpectedWaveLaneCount << "\n";
  }

  if (pInfo1) {
    OS << Comment;
    pInfo1->UsesViewID ? OS << " UsesViewID: true\n"
                       : OS << " UsesViewID: false\n";
    OS << Comment << " SigInputElements: " << (uint32_t)pInfo1->SigInputElements
       << "\n";
    OS << Comment
       << " SigOutputElements: " << (uint32_t)pInfo1->SigOutputElements << "\n";
    OS << Comment << " SigPatchConstOrPrimElements: "
       << (uint32_t)pInfo1->SigPatchConstOrPrimElements << "\n";
    OS << Comment << " SigInputVectors: " << (uint32_t)pInfo1->SigInputVectors
       << "\n";
    for (uint32_t i = 0; i < PSV_GS_MAX_STREAMS; ++i) {
      OS << Comment << " SigOutputVectors[" << i
         << "]: " << (uint32_t)pInfo1->SigOutputVectors[i] << "\n";
    }
  }
  if (pInfo3)
    OS << Comment << " EntryFunctionName: " << EntryName << "\n";
}

void DxilPipelineStateValidation::PrintPSVRuntimeInfo(
    raw_ostream &OS, uint8_t ShaderKind, const char *Comment) const {
  PSVRuntimeInfo0 *pInfo0 = m_pPSVRuntimeInfo0;
  PSVRuntimeInfo1 *pInfo1 = m_pPSVRuntimeInfo1;
  PSVRuntimeInfo2 *pInfo2 = m_pPSVRuntimeInfo2;
  PSVRuntimeInfo3 *pInfo3 = m_pPSVRuntimeInfo3;

  hlsl::PrintPSVRuntimeInfo(
      OS, pInfo0, pInfo1, pInfo2, pInfo3, ShaderKind,
      m_pPSVRuntimeInfo3 ? m_StringTable.Get(pInfo3->EntryFunctionName) : "",
      Comment);
}

void DxilPipelineStateValidation::PrintViewIDState(raw_ostream &OS) const {
  unsigned NumStreams = IsGS() ? PSV_GS_MAX_STREAMS : 1;

  if (m_pPSVRuntimeInfo1->UsesViewID) {
    for (unsigned i = 0; i < NumStreams; ++i) {
      OS << "Outputs affected by ViewID as a bitmask for stream " << i
         << ":\n ";
      uint8_t OutputVectors = m_pPSVRuntimeInfo1->SigOutputVectors[i];
      const PSVComponentMask ViewIDMask(m_pViewIDOutputMask[i], OutputVectors);
      std::string OutputSetName = "Outputs";
      OutputSetName += "[" + std::to_string(i) + "]";
      ViewIDMask.Print(OS, "ViewID", OutputSetName.c_str());
    }

    if (IsHS() || IsMS()) {
      OS << "PCOutputs affected by ViewID as a bitmask:\n";
      uint8_t OutputVectors = m_pPSVRuntimeInfo1->SigPatchConstOrPrimVectors;
      const PSVComponentMask ViewIDMask(m_pViewIDPCOrPrimOutputMask,
                                        OutputVectors);
      ViewIDMask.Print(OS, "ViewID", "PCOutputs");
    }
  }

  for (unsigned i = 0; i < NumStreams; ++i) {
    OS << "Outputs affected by inputs as a table of bitmasks for stream " << i
       << ":\n";
    uint8_t InputVectors = m_pPSVRuntimeInfo1->SigInputVectors;
    uint8_t OutputVectors = m_pPSVRuntimeInfo1->SigOutputVectors[i];
    const PSVDependencyTable Table(m_pInputToOutputTable[i], InputVectors,
                                   OutputVectors);
    std::string OutputSetName = "Outputs";
    OutputSetName += "[" + std::to_string(i) + "]";
    Table.Print(OS, "Inputs", OutputSetName.c_str());
  }

  if (IsHS()) {
    OS << "Patch constant outputs affected by inputs as a table of "
          "bitmasks:\n";
    uint8_t InputVectors = m_pPSVRuntimeInfo1->SigInputVectors;
    uint8_t OutputVectors = m_pPSVRuntimeInfo1->SigPatchConstOrPrimVectors;
    const PSVDependencyTable Table(m_pInputToPCOutputTable, InputVectors,
                                   OutputVectors);
    Table.Print(OS, "Inputs", "PatchConstantOutputs");
  } else if (IsDS()) {
    OS << "Outputs affected by patch constant inputs as a table of "
          "bitmasks:\n";
    uint8_t InputVectors = m_pPSVRuntimeInfo1->SigPatchConstOrPrimVectors;
    uint8_t OutputVectors = m_pPSVRuntimeInfo1->SigOutputVectors[0];
    const PSVDependencyTable Table(m_pPCInputToOutputTable, InputVectors,
                                   OutputVectors);
    Table.Print(OS, "PatchConstantInputs", "Outputs");
  }
}

void DxilPipelineStateValidation::Print(raw_ostream &OS,
                                        uint8_t ShaderKind) const {
  OS << "DxilPipelineStateValidation:\n";
  PrintPSVRuntimeInfo(OS, ShaderKind, "");

  OS << "ResourceCount : " << m_uResourceCount << "\n ";
  if (m_uResourceCount) {
    if (m_uPSVResourceBindInfoSize == sizeof(PSVResourceBindInfo0)) {
      auto *BindInfoPtr = (PSVResourceBindInfo0 *)m_pPSVResourceBindInfo;
      for (uint32_t i = 0; i < m_uResourceCount; ++i)
        (BindInfoPtr + i)->Print(OS);
    } else {
      assert(m_uPSVResourceBindInfoSize == sizeof(PSVResourceBindInfo1));
      auto *BindInfoPtr = (PSVResourceBindInfo1 *)m_pPSVResourceBindInfo;
      for (uint32_t i = 0; i < m_uResourceCount; ++i)
        (BindInfoPtr + i)->Print(OS);
    }
  }

  if (m_pPSVRuntimeInfo1 && (!IsCS() && !IsAS())) {
    assert(m_uPSVSignatureElementSize == sizeof(PSVSignatureElement0));
    PSVSignatureElement0 *InputElements =
        (PSVSignatureElement0 *)m_pSigInputElements;
    for (uint32_t i = 0; i < m_pPSVRuntimeInfo1->SigInputElements; ++i) {
      PSVSignatureElement PSVSE(m_StringTable, m_SemanticIndexTable,
                                InputElements + i);
      PSVSE.Print(OS);
    }
    PSVSignatureElement0 *OutputElements =
        (PSVSignatureElement0 *)m_pSigOutputElements;
    for (uint32_t i = 0; i < m_pPSVRuntimeInfo1->SigOutputElements; ++i) {
      PSVSignatureElement PSVSE(m_StringTable, m_SemanticIndexTable,
                                OutputElements + i);
      PSVSE.Print(OS);
    }
    PSVSignatureElement0 *PatchConstOrPrimElements =
        (PSVSignatureElement0 *)m_pSigPatchConstOrPrimElements;
    for (uint32_t i = 0; i < m_pPSVRuntimeInfo1->SigPatchConstOrPrimElements;
         ++i) {
      PSVSignatureElement PSVSE(m_StringTable, m_SemanticIndexTable,
                                PatchConstOrPrimElements + i);
      PSVSE.Print(OS);
    }

    PrintViewIDState(OS);
  }
}
