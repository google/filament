///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilContainerAssembler.cpp                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides support for serializing a module into DXIL container structures. //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilContainerAssembler.h"
#include "dxc/DXIL/DxilCounters.h"
#include "dxc/DXIL/DxilEntryProps.h"
#include "dxc/DXIL/DxilFunctionProps.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/DxilContainer/DxilRDATBuilder.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.impl.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <algorithm>
#include <assert.h> // Needed for DxilPipelineStateValidation.h
#include <functional>

using namespace llvm;
using namespace hlsl;
using namespace hlsl::RDAT;

static_assert((unsigned)PSVShaderKind::Invalid ==
                  (unsigned)DXIL::ShaderKind::Invalid,
              "otherwise, PSVShaderKind enum out of sync.");

DxilProgramSigSemantic
hlsl::SemanticKindToSystemValue(Semantic::Kind kind,
                                DXIL::TessellatorDomain domain) {
  switch (kind) {
  case Semantic::Kind::Arbitrary:
    return DxilProgramSigSemantic::Undefined;
  case Semantic::Kind::VertexID:
    return DxilProgramSigSemantic::VertexID;
  case Semantic::Kind::InstanceID:
    return DxilProgramSigSemantic::InstanceID;
  case Semantic::Kind::Position:
    return DxilProgramSigSemantic::Position;
  case Semantic::Kind::Coverage:
    return DxilProgramSigSemantic::Coverage;
  case Semantic::Kind::InnerCoverage:
    return DxilProgramSigSemantic::InnerCoverage;
  case Semantic::Kind::PrimitiveID:
    return DxilProgramSigSemantic::PrimitiveID;
  case Semantic::Kind::SampleIndex:
    return DxilProgramSigSemantic::SampleIndex;
  case Semantic::Kind::IsFrontFace:
    return DxilProgramSigSemantic::IsFrontFace;
  case Semantic::Kind::RenderTargetArrayIndex:
    return DxilProgramSigSemantic::RenderTargetArrayIndex;
  case Semantic::Kind::ViewPortArrayIndex:
    return DxilProgramSigSemantic::ViewPortArrayIndex;
  case Semantic::Kind::ClipDistance:
    return DxilProgramSigSemantic::ClipDistance;
  case Semantic::Kind::CullDistance:
    return DxilProgramSigSemantic::CullDistance;
  case Semantic::Kind::Barycentrics:
    return DxilProgramSigSemantic::Barycentrics;
  case Semantic::Kind::ShadingRate:
    return DxilProgramSigSemantic::ShadingRate;
  case Semantic::Kind::CullPrimitive:
    return DxilProgramSigSemantic::CullPrimitive;
  case Semantic::Kind::TessFactor: {
    switch (domain) {
    case DXIL::TessellatorDomain::IsoLine:
      // Will bu updated to DetailTessFactor in next row.
      return DxilProgramSigSemantic::FinalLineDensityTessfactor;
    case DXIL::TessellatorDomain::Tri:
      return DxilProgramSigSemantic::FinalTriEdgeTessfactor;
    case DXIL::TessellatorDomain::Quad:
      return DxilProgramSigSemantic::FinalQuadEdgeTessfactor;
    default:
      // No other valid TesselatorDomain options.
      return DxilProgramSigSemantic::Undefined;
    }
  }
  case Semantic::Kind::InsideTessFactor: {
    switch (domain) {
    case DXIL::TessellatorDomain::IsoLine:
      DXASSERT(0, "invalid semantic");
      return DxilProgramSigSemantic::Undefined;
    case DXIL::TessellatorDomain::Tri:
      return DxilProgramSigSemantic::FinalTriInsideTessfactor;
    case DXIL::TessellatorDomain::Quad:
      return DxilProgramSigSemantic::FinalQuadInsideTessfactor;
    default:
      // No other valid DxilProgramSigSemantic options.
      return DxilProgramSigSemantic::Undefined;
    }
  }
  case Semantic::Kind::Invalid:
    return DxilProgramSigSemantic::Undefined;
  case Semantic::Kind::Target:
    return DxilProgramSigSemantic::Target;
  case Semantic::Kind::Depth:
    return DxilProgramSigSemantic::Depth;
  case Semantic::Kind::DepthLessEqual:
    return DxilProgramSigSemantic::DepthLE;
  case Semantic::Kind::DepthGreaterEqual:
    return DxilProgramSigSemantic::DepthGE;
  case Semantic::Kind::StencilRef:
    LLVM_FALLTHROUGH;
  default:
    DXASSERT(kind == Semantic::Kind::StencilRef,
             "else Invalid or switch is missing a case");
    return DxilProgramSigSemantic::StencilRef;
  }
  // TODO: Final_* values need mappings
}

DxilProgramSigCompType hlsl::CompTypeToSigCompType(hlsl::CompType::Kind Kind,
                                                   bool i1ToUnknownCompat) {
  switch (Kind) {
  case CompType::Kind::I32:
    return DxilProgramSigCompType::SInt32;

  case CompType::Kind::I1:
    // Validator 1.4 and below returned Unknown for i1
    if (i1ToUnknownCompat)
      return DxilProgramSigCompType::Unknown;
    else
      return DxilProgramSigCompType::UInt32;

  case CompType::Kind::U32:
    return DxilProgramSigCompType::UInt32;
  case CompType::Kind::F32:
    return DxilProgramSigCompType::Float32;
  case CompType::Kind::I16:
    return DxilProgramSigCompType::SInt16;
  case CompType::Kind::I64:
    return DxilProgramSigCompType::SInt64;
  case CompType::Kind::U16:
    return DxilProgramSigCompType::UInt16;
  case CompType::Kind::U64:
    return DxilProgramSigCompType::UInt64;
  case CompType::Kind::F16:
    return DxilProgramSigCompType::Float16;
  case CompType::Kind::F64:
    return DxilProgramSigCompType::Float64;
  case CompType::Kind::Invalid:
    LLVM_FALLTHROUGH;
  default:
    return DxilProgramSigCompType::Unknown;
  }
}

static DxilProgramSigMinPrecision
CompTypeToSigMinPrecision(hlsl::CompType value) {
  switch (value.GetKind()) {
  case CompType::Kind::I32:
    return DxilProgramSigMinPrecision::Default;
  case CompType::Kind::U32:
    return DxilProgramSigMinPrecision::Default;
  case CompType::Kind::F32:
    return DxilProgramSigMinPrecision::Default;
  case CompType::Kind::I1:
    return DxilProgramSigMinPrecision::Default;
  case CompType::Kind::U64:
    LLVM_FALLTHROUGH;
  case CompType::Kind::I64:
    LLVM_FALLTHROUGH;
  case CompType::Kind::F64:
    return DxilProgramSigMinPrecision::Default;
  case CompType::Kind::I16:
    return DxilProgramSigMinPrecision::SInt16;
  case CompType::Kind::U16:
    return DxilProgramSigMinPrecision::UInt16;
  case CompType::Kind::F16:
    return DxilProgramSigMinPrecision::Float16; // Float2_8 is not supported in
                                                // DXIL.
  case CompType::Kind::Invalid:
    LLVM_FALLTHROUGH;
  default:
    return DxilProgramSigMinPrecision::Default;
  }
}

template <typename T> struct sort_second {
  bool operator()(const T &a, const T &b) {
    return std::less<decltype(a.second)>()(a.second, b.second);
  }
};

struct sort_sig {
  bool operator()(const DxilProgramSignatureElement &a,
                  const DxilProgramSignatureElement &b) {
    return (a.Stream < b.Stream) ||
           ((a.Stream == b.Stream) && (a.Register < b.Register)) ||
           ((a.Stream == b.Stream) && (a.Register == b.Register) &&
            (a.SemanticName < b.SemanticName));
  }
};

static uint8_t NegMask(uint8_t V) {
  V ^= 0xF;
  return V & 0xF;
}

class DxilProgramSignatureWriter : public DxilPartWriter {
private:
  const DxilSignature &m_signature;
  DXIL::TessellatorDomain m_domain;
  bool m_isInput;
  bool m_useMinPrecision;
  bool m_bCompat_1_4;
  bool m_bCompat_1_6; // unaligned size, no dedup for < 1.7
  size_t m_fixedSize;
  typedef std::pair<const char *, uint32_t> NameOffsetPair_nodedup;
  typedef llvm::SmallMapVector<const char *, uint32_t, 8> NameOffsetMap_nodedup;
  typedef std::pair<llvm::StringRef, uint32_t> NameOffsetPair;
  typedef llvm::SmallMapVector<llvm::StringRef, uint32_t, 8> NameOffsetMap;
  uint32_t m_lastOffset;
  NameOffsetMap_nodedup m_semanticNameOffsets_nodedup;
  NameOffsetMap m_semanticNameOffsets;
  unsigned m_paramCount;

  const char *GetSemanticName(const hlsl::DxilSignatureElement *pElement) {
    DXASSERT_NOMSG(pElement != nullptr);
    DXASSERT(pElement->GetName() != nullptr, "else sig is malformed");
    return pElement->GetName();
  }

  uint32_t
  GetSemanticOffset_nodedup(const hlsl::DxilSignatureElement *pElement) {
    const char *pName = GetSemanticName(pElement);
    NameOffsetMap_nodedup::iterator nameOffset =
        m_semanticNameOffsets_nodedup.find(pName);
    uint32_t result;
    if (nameOffset == m_semanticNameOffsets_nodedup.end()) {
      result = m_lastOffset;
      m_semanticNameOffsets_nodedup.insert(
          NameOffsetPair_nodedup(pName, result));
      m_lastOffset += strlen(pName) + 1;
    } else {
      result = nameOffset->second;
    }
    return result;
  }
  uint32_t GetSemanticOffset(const hlsl::DxilSignatureElement *pElement) {
    if (m_bCompat_1_6)
      return GetSemanticOffset_nodedup(pElement);

    StringRef name = GetSemanticName(pElement);
    NameOffsetMap::iterator nameOffset = m_semanticNameOffsets.find(name);
    uint32_t result;
    if (nameOffset == m_semanticNameOffsets.end()) {
      result = m_lastOffset;
      m_semanticNameOffsets.insert(NameOffsetPair(name, result));
      m_lastOffset += name.size() + 1;
    } else {
      result = nameOffset->second;
    }
    return result;
  }

  void write(std::vector<DxilProgramSignatureElement> &orderedSig,
             const hlsl::DxilSignatureElement *pElement) {
    const std::vector<unsigned> &indexVec = pElement->GetSemanticIndexVec();
    unsigned eltCount = pElement->GetSemanticIndexVec().size();
    unsigned eltRows = 1;
    if (eltCount)
      eltRows = pElement->GetRows() / eltCount;
    DXASSERT_NOMSG(eltRows == 1);

    DxilProgramSignatureElement sig;
    memset(&sig, 0, sizeof(DxilProgramSignatureElement));
    sig.Stream = pElement->GetOutputStream();
    sig.SemanticName = GetSemanticOffset(pElement);
    sig.SystemValue = SemanticKindToSystemValue(pElement->GetKind(), m_domain);
    sig.CompType =
        CompTypeToSigCompType(pElement->GetCompType().GetKind(), m_bCompat_1_4);
    sig.Register = pElement->GetStartRow();

    sig.Mask = pElement->GetColsAsMask();
    if (m_bCompat_1_4) {
      // Match what validator 1.4 and below expects
      // Only mark exist channel write for output.
      // All channel not used for input.
      if (!m_isInput)
        sig.NeverWrites_Mask = ~sig.Mask;
      else
        sig.AlwaysReads_Mask = 0;
    } else {
      unsigned UsageMask = pElement->GetUsageMask();
      if (pElement->IsAllocated())
        UsageMask <<= pElement->GetStartCol();
      if (!m_isInput)
        sig.NeverWrites_Mask = NegMask(UsageMask);
      else
        sig.AlwaysReads_Mask = UsageMask;
    }

    sig.MinPrecision = m_useMinPrecision
                           ? CompTypeToSigMinPrecision(pElement->GetCompType())
                           : DxilProgramSigMinPrecision::Default;

    for (unsigned i = 0; i < eltCount; ++i) {
      sig.SemanticIndex = indexVec[i];
      orderedSig.emplace_back(sig);
      if (pElement->IsAllocated())
        sig.Register += eltRows;
      if (sig.SystemValue == DxilProgramSigSemantic::FinalLineDensityTessfactor)
        sig.SystemValue = DxilProgramSigSemantic::FinalLineDetailTessfactor;
    }
  }

  void calcSizes() {
    // Calculate size for signature elements.
    const std::vector<std::unique_ptr<hlsl::DxilSignatureElement>> &elements =
        m_signature.GetElements();
    uint32_t result = sizeof(DxilProgramSignature);
    m_paramCount = 0;
    for (size_t i = 0; i < elements.size(); ++i) {
      DXIL::SemanticInterpretationKind I = elements[i]->GetInterpretation();
      if (I == DXIL::SemanticInterpretationKind::NA ||
          I == DXIL::SemanticInterpretationKind::NotInSig)
        continue;
      unsigned semanticCount = elements[i]->GetSemanticIndexVec().size();
      result += semanticCount * sizeof(DxilProgramSignatureElement);
      m_paramCount += semanticCount;
    }
    m_fixedSize = result;
    m_lastOffset = m_fixedSize;

    // Calculate size for semantic strings.
    for (size_t i = 0; i < elements.size(); ++i) {
      GetSemanticOffset(elements[i].get());
    }
  }

public:
  DxilProgramSignatureWriter(const DxilSignature &signature,
                             DXIL::TessellatorDomain domain, bool isInput,
                             bool UseMinPrecision, bool bCompat_1_4,
                             bool bCompat_1_6)
      : m_signature(signature), m_domain(domain), m_isInput(isInput),
        m_useMinPrecision(UseMinPrecision), m_bCompat_1_4(bCompat_1_4),
        m_bCompat_1_6(bCompat_1_6) {
    calcSizes();
  }

  uint32_t size() const override {
    if (m_bCompat_1_6)
      return m_lastOffset;
    else
      return PSVALIGN4(m_lastOffset);
  }

  void write(AbstractMemoryStream *pStream) override {
    UINT64 startPos = pStream->GetPosition();
    const std::vector<std::unique_ptr<hlsl::DxilSignatureElement>> &elements =
        m_signature.GetElements();

    DxilProgramSignature programSig;
    programSig.ParamCount = m_paramCount;
    programSig.ParamOffset = sizeof(DxilProgramSignature);
    IFT(WriteStreamValue(pStream, programSig));

    // Write structures in register order.
    std::vector<DxilProgramSignatureElement> orderedSig;
    for (size_t i = 0; i < elements.size(); ++i) {
      DXIL::SemanticInterpretationKind I = elements[i]->GetInterpretation();
      if (I == DXIL::SemanticInterpretationKind::NA ||
          I == DXIL::SemanticInterpretationKind::NotInSig)
        continue;
      write(orderedSig, elements[i].get());
    }
    std::sort(orderedSig.begin(), orderedSig.end(), sort_sig());
    for (size_t i = 0; i < orderedSig.size(); ++i) {
      DxilProgramSignatureElement &sigElt = orderedSig[i];
      IFT(WriteStreamValue(pStream, sigElt));
    }

    // Write strings in the offset order.
    std::vector<NameOffsetPair> ordered;
    if (m_bCompat_1_6) {
      ordered.assign(m_semanticNameOffsets_nodedup.begin(),
                     m_semanticNameOffsets_nodedup.end());
    } else {
      ordered.assign(m_semanticNameOffsets.begin(),
                     m_semanticNameOffsets.end());
    }
    std::sort(ordered.begin(), ordered.end(), sort_second<NameOffsetPair>());
    for (size_t i = 0; i < ordered.size(); ++i) {
      StringRef name = ordered[i].first;
      ULONG cbWritten;
      UINT64 offsetPos = pStream->GetPosition();
      DXASSERT_LOCALVAR(offsetPos, offsetPos - startPos == ordered[i].second,
                        "else str offset is incorrect");
      IFT(pStream->Write(name.data(), name.size() + 1, &cbWritten));
    }

    // Align, and verify we wrote the same number of bytes we though we would.
    UINT64 bytesWritten = pStream->GetPosition() - startPos;
    if (!m_bCompat_1_6 && (bytesWritten % 4 != 0)) {
      unsigned paddingToAdd = 4 - (bytesWritten % 4);
      char padding[4] = {0};
      ULONG cbWritten = 0;
      IFT(pStream->Write(padding, paddingToAdd, &cbWritten));
      bytesWritten += cbWritten;
    }
    DXASSERT(bytesWritten == size(), "else size is incorrect");
  }
};

DxilPartWriter *hlsl::NewProgramSignatureWriter(const DxilModule &M,
                                                DXIL::SignatureKind Kind) {
  DXIL::TessellatorDomain domain = DXIL::TessellatorDomain::Undefined;
  if (M.GetShaderModel()->IsHS() || M.GetShaderModel()->IsDS())
    domain = M.GetTessellatorDomain();
  unsigned ValMajor, ValMinor;
  M.GetValidatorVersion(ValMajor, ValMinor);
  bool bCompat_1_4 = DXIL::CompareVersions(ValMajor, ValMinor, 1, 5) < 0;
  bool bCompat_1_6 = DXIL::CompareVersions(ValMajor, ValMinor, 1, 7) < 0;
  switch (Kind) {
  case DXIL::SignatureKind::Input:
    return new DxilProgramSignatureWriter(M.GetInputSignature(), domain, true,
                                          M.GetUseMinPrecision(), bCompat_1_4,
                                          bCompat_1_6);
  case DXIL::SignatureKind::Output:
    return new DxilProgramSignatureWriter(M.GetOutputSignature(), domain, false,
                                          M.GetUseMinPrecision(), bCompat_1_4,
                                          bCompat_1_6);
  case DXIL::SignatureKind::PatchConstOrPrim:
    return new DxilProgramSignatureWriter(
        M.GetPatchConstOrPrimSignature(), domain,
        /*IsInput*/ M.GetShaderModel()->IsDS(),
        /*UseMinPrecision*/ M.GetUseMinPrecision(), bCompat_1_4, bCompat_1_6);
  case DXIL::SignatureKind::Invalid:
    return nullptr;
  }
  return nullptr;
}

class DxilProgramRootSignatureWriter : public DxilPartWriter {
private:
  const RootSignatureHandle &m_Sig;

public:
  DxilProgramRootSignatureWriter(const RootSignatureHandle &S) : m_Sig(S) {}
  uint32_t size() const { return m_Sig.GetSerializedSize(); }
  void write(AbstractMemoryStream *pStream) {
    ULONG cbWritten;
    IFT(pStream->Write(m_Sig.GetSerializedBytes(), size(), &cbWritten));
  }
};

DxilPartWriter *hlsl::NewRootSignatureWriter(const RootSignatureHandle &S) {
  return new DxilProgramRootSignatureWriter(S);
}

class DxilFeatureInfoWriter : public DxilPartWriter {
private:
  // Only save the shader properties after create class for it.
  DxilShaderFeatureInfo featureInfo;

public:
  DxilFeatureInfoWriter(const DxilModule &M) {
    featureInfo.FeatureFlags = M.m_ShaderFlags.GetFeatureInfo();
  }
  uint32_t size() const override { return sizeof(DxilShaderFeatureInfo); }
  void write(AbstractMemoryStream *pStream) override {
    IFT(WriteStreamValue(pStream, featureInfo.FeatureFlags));
  }
};

DxilPartWriter *hlsl::NewFeatureInfoWriter(const DxilModule &M) {
  return new DxilFeatureInfoWriter(M);
}

//////////////////////////////////////////////////////////
// Utility code for serializing/deserializing ViewID state

// Code for ComputeSeriaizedViewIDStateSizeInUInts copied from
// ComputeViewIdState. It could be moved into some common location if this
// ViewID serialization/deserialization code were moved out of here.
static unsigned RoundUpToUINT(unsigned x) { return (x + 31) / 32; }
static unsigned ComputeSeriaizedViewIDStateSizeInUInts(
    const PSVShaderKind SK, const bool bUsesViewID, const unsigned InputScalars,
    const unsigned OutputScalars[4], const unsigned PCScalars) {
  // Compute serialized state size in UINTs.
  unsigned NumStreams = SK == PSVShaderKind::Geometry ? 4 : 1;
  unsigned Size = 0;
  Size += 1; // #Inputs.
  for (unsigned StreamId = 0; StreamId < NumStreams; StreamId++) {
    Size += 1; // #Outputs for stream StreamId.
    unsigned NumOutputs = OutputScalars[StreamId];
    unsigned NumOutUINTs = RoundUpToUINT(NumOutputs);
    if (bUsesViewID) {
      Size += NumOutUINTs; // m_OutputsDependentOnViewId[StreamId]
    }
    Size +=
        InputScalars * NumOutUINTs; // m_InputsContributingToOutputs[StreamId]
  }
  if (SK == PSVShaderKind::Hull || SK == PSVShaderKind::Domain ||
      SK == PSVShaderKind::Mesh) {
    Size += 1; // #PatchConstant.
    unsigned NumPCUINTs = RoundUpToUINT(PCScalars);
    if (SK == PSVShaderKind::Hull || SK == PSVShaderKind::Mesh) {
      if (bUsesViewID) {
        Size += NumPCUINTs; // m_PCOrPrimOutputsDependentOnViewId
      }
      Size +=
          InputScalars * NumPCUINTs; // m_InputsContributingToPCOrPrimOutputs
    } else {
      unsigned NumOutputs = OutputScalars[0];
      unsigned NumOutUINTs = RoundUpToUINT(NumOutputs);
      Size += PCScalars * NumOutUINTs; // m_PCInputsContributingToOutputs
    }
  }
  return Size;
}

static const uint32_t *CopyViewIDStateForOutputToPSV(
    const uint32_t *pSrc, uint32_t InputScalars, uint32_t OutputScalars,
    PSVComponentMask ViewIDMask, PSVDependencyTable IOTable) {
  unsigned MaskDwords =
      PSVComputeMaskDwordsFromVectors(PSVALIGN4(OutputScalars) / 4);
  if (ViewIDMask.IsValid()) {
    DXASSERT_NOMSG(!IOTable.Table ||
                   ViewIDMask.NumVectors == IOTable.OutputVectors);
    memcpy(ViewIDMask.Mask, pSrc, 4 * MaskDwords);
    pSrc += MaskDwords;
  }
  if (IOTable.IsValid() && IOTable.InputVectors && IOTable.OutputVectors) {
    DXASSERT_NOMSG((InputScalars <= IOTable.InputVectors * 4) &&
                   (IOTable.InputVectors * 4 - InputScalars < 4));
    DXASSERT_NOMSG((OutputScalars <= IOTable.OutputVectors * 4) &&
                   (IOTable.OutputVectors * 4 - OutputScalars < 4));
    memcpy(IOTable.Table, pSrc, 4 * MaskDwords * InputScalars);
    pSrc += MaskDwords * InputScalars;
  }
  return pSrc;
}

static uint32_t *CopyViewIDStateForOutputFromPSV(uint32_t *pOutputData,
                                                 const unsigned InputScalars,
                                                 const unsigned OutputScalars,
                                                 PSVComponentMask ViewIDMask,
                                                 PSVDependencyTable IOTable) {
  unsigned MaskDwords =
      PSVComputeMaskDwordsFromVectors(PSVALIGN4(OutputScalars) / 4);
  if (ViewIDMask.IsValid()) {
    DXASSERT_NOMSG(!IOTable.Table ||
                   ViewIDMask.NumVectors == IOTable.OutputVectors);
    for (unsigned i = 0; i < MaskDwords; i++)
      *(pOutputData++) = ViewIDMask.Mask[i];
  }
  if (IOTable.IsValid() && IOTable.InputVectors && IOTable.OutputVectors) {
    DXASSERT_NOMSG((InputScalars <= IOTable.InputVectors * 4) &&
                   (IOTable.InputVectors * 4 - InputScalars < 4));
    DXASSERT_NOMSG((OutputScalars <= IOTable.OutputVectors * 4) &&
                   (IOTable.OutputVectors * 4 - OutputScalars < 4));
    for (unsigned i = 0; i < MaskDwords * InputScalars; i++)
      *(pOutputData++) = IOTable.Table[i];
  }
  return pOutputData;
}

void hlsl::StoreViewIDStateToPSV(const uint32_t *pInputData,
                                 unsigned InputSizeInUInts,
                                 DxilPipelineStateValidation &PSV) {
  PSVRuntimeInfo1 *pInfo1 = PSV.GetPSVRuntimeInfo1();
  DXASSERT(pInfo1, "otherwise, PSV does not meet version requirement.");
  PSVShaderKind SK = static_cast<PSVShaderKind>(pInfo1->ShaderStage);
  const unsigned OutputStreams = SK == PSVShaderKind::Geometry ? 4 : 1;
  const uint32_t *pSrc = pInputData;
  const uint32_t InputScalars = *(pSrc++);
  uint32_t OutputScalars[4];
  for (unsigned streamIndex = 0; streamIndex < OutputStreams; streamIndex++) {
    OutputScalars[streamIndex] = *(pSrc++);
    pSrc = CopyViewIDStateForOutputToPSV(
        pSrc, InputScalars, OutputScalars[streamIndex],
        PSV.GetViewIDOutputMask(streamIndex),
        PSV.GetInputToOutputTable(streamIndex));
  }
  if (SK == PSVShaderKind::Hull || SK == PSVShaderKind::Mesh) {
    const uint32_t PCScalars = *(pSrc++);
    pSrc = CopyViewIDStateForOutputToPSV(pSrc, InputScalars, PCScalars,
                                         PSV.GetViewIDPCOutputMask(),
                                         PSV.GetInputToPCOutputTable());
  } else if (SK == PSVShaderKind::Domain) {
    const uint32_t PCScalars = *(pSrc++);
    pSrc = CopyViewIDStateForOutputToPSV(pSrc, PCScalars, OutputScalars[0],
                                         PSVComponentMask(),
                                         PSV.GetPCInputToOutputTable());
  }
  DXASSERT((unsigned)(pSrc - pInputData) == InputSizeInUInts,
           "otherwise, different amout of data written than expected.");
}

// This function is defined close to the serialization code in DxilPSVWriter to
// reduce the chance of a mismatch.  It could be defined elsewhere, but it would
// make sense to move both the serialization and deserialization out of here and
// into a common location.
unsigned hlsl::LoadViewIDStateFromPSV(unsigned *pOutputData,
                                      unsigned OutputSizeInUInts,
                                      const DxilPipelineStateValidation &PSV) {
  PSVRuntimeInfo1 *pInfo1 = PSV.GetPSVRuntimeInfo1();
  if (!pInfo1) {
    return 0;
  }
  PSVShaderKind SK = static_cast<PSVShaderKind>(pInfo1->ShaderStage);
  const unsigned OutputStreams = SK == PSVShaderKind::Geometry ? 4 : 1;
  const unsigned InputScalars = pInfo1->SigInputVectors * 4;
  unsigned OutputScalars[4];
  for (unsigned streamIndex = 0; streamIndex < OutputStreams; streamIndex++) {
    OutputScalars[streamIndex] = pInfo1->SigOutputVectors[streamIndex] * 4;
  }
  unsigned PCScalars = 0;
  if (SK == PSVShaderKind::Hull || SK == PSVShaderKind::Mesh ||
      SK == PSVShaderKind::Domain) {
    PCScalars = pInfo1->SigPatchConstOrPrimVectors * 4;
  }
  if (pOutputData == nullptr) {
    return ComputeSeriaizedViewIDStateSizeInUInts(
        SK, pInfo1->UsesViewID != 0, InputScalars, OutputScalars, PCScalars);
  }

  // Fill in serialized viewid buffer.
  DXASSERT(ComputeSeriaizedViewIDStateSizeInUInts(
               SK, pInfo1->UsesViewID != 0, InputScalars, OutputScalars,
               PCScalars) == OutputSizeInUInts,
           "otherwise, OutputSize doesn't match computed size.");
  unsigned *pStartOutputData = pOutputData;
  *(pOutputData++) = InputScalars;
  for (unsigned streamIndex = 0; streamIndex < OutputStreams; streamIndex++) {
    *(pOutputData++) = OutputScalars[streamIndex];
    pOutputData = CopyViewIDStateForOutputFromPSV(
        pOutputData, InputScalars, OutputScalars[streamIndex],
        PSV.GetViewIDOutputMask(streamIndex),
        PSV.GetInputToOutputTable(streamIndex));
  }
  if (SK == PSVShaderKind::Hull || SK == PSVShaderKind::Mesh) {
    *(pOutputData++) = PCScalars;
    pOutputData = CopyViewIDStateForOutputFromPSV(
        pOutputData, InputScalars, PCScalars, PSV.GetViewIDPCOutputMask(),
        PSV.GetInputToPCOutputTable());
  } else if (SK == PSVShaderKind::Domain) {
    *(pOutputData++) = PCScalars;
    pOutputData = CopyViewIDStateForOutputFromPSV(
        pOutputData, PCScalars, OutputScalars[0], PSVComponentMask(),
        PSV.GetPCInputToOutputTable());
  }
  DXASSERT((unsigned)(pOutputData - pStartOutputData) == OutputSizeInUInts,
           "otherwise, OutputSizeInUInts didn't match size written.");
  return pOutputData - pStartOutputData;
}

//////////////////////////////////////////////////////////
// DxilPSVWriter - Writes PSV0 part

class DxilPSVWriter : public DxilPartWriter {
private:
  const DxilModule &m_Module;
  unsigned m_ValMajor = 0, m_ValMinor = 0;
  PSVInitInfo m_PSVInitInfo;
  DxilPipelineStateValidation m_PSV;
  uint32_t m_PSVBufferSize = 0;
  SmallVector<char, 512> m_PSVBuffer;
  SmallVector<char, 256> m_StringBuffer;
  SmallVector<uint32_t, 8> m_SemanticIndexBuffer;
  std::vector<PSVSignatureElement0> m_SigInputElements;
  std::vector<PSVSignatureElement0> m_SigOutputElements;
  std::vector<PSVSignatureElement0> m_SigPatchConstOrPrimElements;
  unsigned EntryFunctionName = 0;

  void SetPSVSigElement(PSVSignatureElement0 &E,
                        const DxilSignatureElement &SE) {
    memset(&E, 0, sizeof(PSVSignatureElement0));
    bool i1ToUnknownCompat =
        DXIL::CompareVersions(m_ValMajor, m_ValMinor, 1, 5) < 0;
    InitPSVSignatureElement(E, SE, i1ToUnknownCompat);
    // Setup semantic name.
    if (SE.GetKind() == DXIL::SemanticKind::Arbitrary &&
        strlen(SE.GetName()) > 0) {
      E.SemanticName = (uint32_t)m_StringBuffer.size();
      StringRef Name(SE.GetName());
      m_StringBuffer.append(Name.size() + 1, '\0');
      memcpy(m_StringBuffer.data() + E.SemanticName, Name.data(), Name.size());
    } else {
      // m_StringBuffer always starts with '\0' so offset 0 is empty string:
      E.SemanticName = 0;
    }
    // Search index buffer for matching semantic index sequence
    DXASSERT_NOMSG(SE.GetRows() == SE.GetSemanticIndexVec().size());
    auto &SemIdx = SE.GetSemanticIndexVec();
    bool match = false;
    for (uint32_t offset = 0;
         offset + SE.GetRows() - 1 < m_SemanticIndexBuffer.size(); offset++) {
      match = true;
      for (uint32_t row = 0; row < SE.GetRows(); row++) {
        if ((uint32_t)SemIdx[row] != m_SemanticIndexBuffer[offset + row]) {
          match = false;
          break;
        }
      }
      if (match) {
        E.SemanticIndexes = offset;
        break;
      }
    }
    if (!match) {
      E.SemanticIndexes = m_SemanticIndexBuffer.size();
      for (uint32_t row = 0; row < SemIdx.size(); row++) {
        m_SemanticIndexBuffer.push_back((uint32_t)SemIdx[row]);
      }
    }
  }

public:
  DxilPSVWriter(const DxilModule &mod, uint32_t PSVVersion = UINT_MAX)
      : m_Module(mod), m_PSVInitInfo(PSVVersion) {
    m_Module.GetValidatorVersion(m_ValMajor, m_ValMinor);
    hlsl::SetupPSVInitInfo(m_PSVInitInfo, m_Module);

    // TODO: for >= 6.2 version, create more efficient structure
    if (m_PSVInitInfo.PSVVersion > 0) {
      // Copy Dxil Signatures
      m_StringBuffer.push_back('\0'); // For empty semantic name (system value)
      m_SigInputElements.resize(m_PSVInitInfo.SigInputElements);
      m_SigOutputElements.resize(m_PSVInitInfo.SigOutputElements);
      m_SigPatchConstOrPrimElements.resize(
          m_PSVInitInfo.SigPatchConstOrPrimElements);
      uint32_t i = 0;
      for (auto &SE : m_Module.GetInputSignature().GetElements()) {
        SetPSVSigElement(m_SigInputElements[i++], *(SE.get()));
      }
      i = 0;
      for (auto &SE : m_Module.GetOutputSignature().GetElements()) {
        SetPSVSigElement(m_SigOutputElements[i++], *(SE.get()));
      }
      i = 0;
      for (auto &SE : m_Module.GetPatchConstOrPrimSignature().GetElements()) {
        SetPSVSigElement(m_SigPatchConstOrPrimElements[i++], *(SE.get()));
      }

      // Add entry function name to string table in version 3 and above.
      if (m_PSVInitInfo.PSVVersion > 2) {
        EntryFunctionName = (uint32_t)m_StringBuffer.size();
        StringRef Name(m_Module.GetEntryFunctionName());
        m_StringBuffer.append(Name.size() + 1, '\0');
        memcpy(m_StringBuffer.data() + EntryFunctionName, Name.data(),
               Name.size());
      }

      // Set String and SemanticInput Tables
      m_PSVInitInfo.StringTable.Table = m_StringBuffer.data();
      m_PSVInitInfo.StringTable.Size = m_StringBuffer.size();
      m_PSVInitInfo.SemanticIndexTable.Table = m_SemanticIndexBuffer.data();
      m_PSVInitInfo.SemanticIndexTable.Entries = m_SemanticIndexBuffer.size();
    }
    if (!m_PSV.InitNew(m_PSVInitInfo, nullptr, &m_PSVBufferSize)) {
      DXASSERT(false, "PSV InitNew failed computing size!");
    }
  }
  uint32_t size() const override { return m_PSVBufferSize; }

  void write(AbstractMemoryStream *pStream) override {
    // Do not add any data in write() which wasn't accounted for already in the
    // constructor, where we compute the size based on m_PSVInitInfo.

    m_PSVBuffer.resize(m_PSVBufferSize);
    if (!m_PSV.InitNew(m_PSVInitInfo, m_PSVBuffer.data(), &m_PSVBufferSize)) {
      DXASSERT(false, "PSV InitNew failed!");
    }
    DXASSERT_NOMSG(m_PSVBuffer.size() == m_PSVBufferSize);

    // Set DxilRuntimeInfo
    PSVRuntimeInfo0 *pInfo = m_PSV.GetPSVRuntimeInfo0();
    PSVRuntimeInfo1 *pInfo1 = m_PSV.GetPSVRuntimeInfo1();
    PSVRuntimeInfo2 *pInfo2 = m_PSV.GetPSVRuntimeInfo2();
    PSVRuntimeInfo3 *pInfo3 = m_PSV.GetPSVRuntimeInfo3();
    if (pInfo)
      hlsl::SetShaderProps(pInfo, m_Module);
    if (pInfo1)
      hlsl::SetShaderProps(pInfo1, m_Module);
    if (pInfo2)
      hlsl::SetShaderProps(pInfo2, m_Module);
    if (pInfo3)
      pInfo3->EntryFunctionName = EntryFunctionName;

    // Set resource binding information
    UINT uResIndex = 0;
    for (auto &&R : m_Module.GetCBuffers()) {
      DXASSERT_NOMSG(uResIndex < m_PSVInitInfo.ResourceCount);
      PSVResourceBindInfo0 *pBindInfo =
          m_PSV.GetPSVResourceBindInfo0(uResIndex);
      PSVResourceBindInfo1 *pBindInfo1 =
          m_PSV.GetPSVResourceBindInfo1(uResIndex);
      DXASSERT_NOMSG(pBindInfo);
      InitPSVResourceBinding(pBindInfo, pBindInfo1, R.get());
      uResIndex++;
    }
    for (auto &&R : m_Module.GetSamplers()) {
      DXASSERT_NOMSG(uResIndex < m_PSVInitInfo.ResourceCount);
      PSVResourceBindInfo0 *pBindInfo =
          m_PSV.GetPSVResourceBindInfo0(uResIndex);
      PSVResourceBindInfo1 *pBindInfo1 =
          m_PSV.GetPSVResourceBindInfo1(uResIndex);
      DXASSERT_NOMSG(pBindInfo);
      InitPSVResourceBinding(pBindInfo, pBindInfo1, R.get());
      uResIndex++;
    }
    for (auto &&R : m_Module.GetSRVs()) {
      DXASSERT_NOMSG(uResIndex < m_PSVInitInfo.ResourceCount);
      PSVResourceBindInfo0 *pBindInfo =
          m_PSV.GetPSVResourceBindInfo0(uResIndex);
      PSVResourceBindInfo1 *pBindInfo1 =
          m_PSV.GetPSVResourceBindInfo1(uResIndex);
      DXASSERT_NOMSG(pBindInfo);
      InitPSVResourceBinding(pBindInfo, pBindInfo1, R.get());
      uResIndex++;
    }
    for (auto &&R : m_Module.GetUAVs()) {
      DXASSERT_NOMSG(uResIndex < m_PSVInitInfo.ResourceCount);
      PSVResourceBindInfo0 *pBindInfo =
          m_PSV.GetPSVResourceBindInfo0(uResIndex);
      PSVResourceBindInfo1 *pBindInfo1 =
          m_PSV.GetPSVResourceBindInfo1(uResIndex);
      DXASSERT_NOMSG(pBindInfo);
      InitPSVResourceBinding(pBindInfo, pBindInfo1, R.get());
      uResIndex++;
    }
    DXASSERT_NOMSG(uResIndex == m_PSVInitInfo.ResourceCount);

    if (m_PSVInitInfo.PSVVersion > 0) {
      DXASSERT_NOMSG(pInfo1);

      // Write Dxil Signature Elements
      for (unsigned i = 0; i < m_PSV.GetSigInputElements(); i++) {
        PSVSignatureElement0 *pInputElement = m_PSV.GetInputElement0(i);
        DXASSERT_NOMSG(pInputElement);
        memcpy(pInputElement, &m_SigInputElements[i],
               sizeof(PSVSignatureElement0));
      }
      for (unsigned i = 0; i < m_PSV.GetSigOutputElements(); i++) {
        PSVSignatureElement0 *pOutputElement = m_PSV.GetOutputElement0(i);
        DXASSERT_NOMSG(pOutputElement);
        memcpy(pOutputElement, &m_SigOutputElements[i],
               sizeof(PSVSignatureElement0));
      }
      for (unsigned i = 0; i < m_PSV.GetSigPatchConstOrPrimElements(); i++) {
        PSVSignatureElement0 *pPatchConstOrPrimElement =
            m_PSV.GetPatchConstOrPrimElement0(i);
        DXASSERT_NOMSG(pPatchConstOrPrimElement);
        memcpy(pPatchConstOrPrimElement, &m_SigPatchConstOrPrimElements[i],
               sizeof(PSVSignatureElement0));
      }

      // Gather ViewID dependency information
      auto &viewState = m_Module.GetSerializedViewIdState();
      if (!viewState.empty()) {
        StoreViewIDStateToPSV(viewState.data(), (unsigned)viewState.size(),
                              m_PSV);
      }
    }

    // Ensure that these buffers were not modified after m_PSVInitInfo was set.
    DXASSERT((uint32_t)m_StringBuffer.size() == m_PSVInitInfo.StringTable.Size,
             "otherwise m_StringBuffer modified after m_PSVInitInfo set.");
    DXASSERT(
        (uint32_t)m_SemanticIndexBuffer.size() ==
            m_PSVInitInfo.SemanticIndexTable.Entries,
        "otherwise m_SemanticIndexBuffer modified after m_PSVInitInfo set.");

    ULONG cbWritten;
    IFT(pStream->Write(m_PSVBuffer.data(), m_PSVBufferSize, &cbWritten));
    DXASSERT_NOMSG(cbWritten == m_PSVBufferSize);
  }
};

//////////////////////////////////////////////////////////
// DxilVersionWriter - Writes VERS part
class DxilVersionWriter : public DxilPartWriter {
  hlsl::DxilCompilerVersion m_Header = {};
  CComHeapPtr<char> m_CommitShaStorage;
  llvm::StringRef m_CommitSha = "";
  CComHeapPtr<char> m_CustomStringStorage;
  llvm::StringRef m_CustomString = "";

public:
  DxilVersionWriter(IDxcVersionInfo *pVersion) { Init(pVersion); }

  void Init(IDxcVersionInfo *pVersionInfo) {
    m_Header = {};

    UINT32 Major = 0, Minor = 0;
    UINT32 Flags = 0;
    IFT(pVersionInfo->GetVersion(&Major, &Minor));
    IFT(pVersionInfo->GetFlags(&Flags));

    m_Header.Major = Major;
    m_Header.Minor = Minor;
    m_Header.VersionFlags = Flags;
    CComPtr<IDxcVersionInfo2> pVersionInfo2;
    if (SUCCEEDED(pVersionInfo->QueryInterface(&pVersionInfo2))) {
      UINT32 CommitCount = 0;
      IFT(pVersionInfo2->GetCommitInfo(&CommitCount, &m_CommitShaStorage));
      m_CommitSha = llvm::StringRef(m_CommitShaStorage.m_pData,
                                    strlen(m_CommitShaStorage.m_pData));
      m_Header.CommitCount = CommitCount;
      m_Header.VersionStringListSizeInBytes += m_CommitSha.size();
    }
    m_Header.VersionStringListSizeInBytes += /*null term*/ 1;

    CComPtr<IDxcVersionInfo3> pVersionInfo3;
    if (SUCCEEDED(pVersionInfo->QueryInterface(&pVersionInfo3))) {
      IFT(pVersionInfo3->GetCustomVersionString(&m_CustomStringStorage));
      m_CustomString = llvm::StringRef(m_CustomStringStorage,
                                       strlen(m_CustomStringStorage.m_pData));
      m_Header.VersionStringListSizeInBytes += m_CustomString.size();
    }
    m_Header.VersionStringListSizeInBytes += /*null term*/ 1;
  }

  static uint32_t PadToDword(uint32_t size, uint32_t *outNumPadding = nullptr) {
    uint32_t rem = size % 4;
    if (rem) {
      uint32_t padding = (4 - rem);
      if (outNumPadding)
        *outNumPadding = padding;
      return size + padding;
    }
    if (outNumPadding)
      *outNumPadding = 0;
    return size;
  }

  UINT32 size() const override {
    return PadToDword(sizeof(m_Header) + m_Header.VersionStringListSizeInBytes);
  }

  void write(AbstractMemoryStream *pStream) override {
    const uint8_t padByte = 0;
    UINT32 uPadding = 0;
    UINT32 uSize = PadToDword(
        sizeof(m_Header) + m_Header.VersionStringListSizeInBytes, &uPadding);
    (void)uSize;

    ULONG cbWritten = 0;
    IFT(pStream->Write(&m_Header, sizeof(m_Header), &cbWritten));

    // Write a null terminator even if the string is empty
    IFT(pStream->Write(m_CommitSha.data(), m_CommitSha.size(), &cbWritten));
    // Null terminator for the commit sha
    IFT(pStream->Write(&padByte, sizeof(padByte), &cbWritten));

    // Write the custom version string.
    IFT(pStream->Write(m_CustomString.data(), m_CustomString.size(),
                       &cbWritten));
    // Null terminator for the custom version string.
    IFT(pStream->Write(&padByte, sizeof(padByte), &cbWritten));

    // Write padding
    for (unsigned i = 0; i < uPadding; i++) {
      IFT(pStream->Write(&padByte, sizeof(padByte), &cbWritten));
    }
  }
};

using namespace DXIL;

class DxilRDATWriter : public DxilPartWriter {
private:
  DxilRDATBuilder Builder;
  RDATTable *m_pResourceTable;
  RDATTable *m_pFunctionTable;
  RDATTable *m_pSubobjectTable;

  typedef llvm::SmallSetVector<uint32_t, 8> Indices;
  typedef std::unordered_map<const llvm::Function *, Indices> FunctionIndexMap;
  FunctionIndexMap m_FuncToResNameOffset; // list of resources used
  FunctionIndexMap m_FuncToDependencies;  // list of unresolved functions used

  unsigned m_ValMajor, m_ValMinor;

  void
  FindUsingFunctions(const llvm::Value *User,
                     llvm::SmallVectorImpl<const llvm::Function *> &functions) {
    if (const llvm::Instruction *I = dyn_cast<const llvm::Instruction>(User)) {
      // Instruction should be inside a basic block, which is in a function
      functions.push_back(
          cast<const llvm::Function>(I->getParent()->getParent()));
      return;
    }
    // User can be either instruction, constant, or operator. But User is an
    // operator only if constant is a scalar value, not resource pointer.
    const llvm::Constant *CU = cast<const llvm::Constant>(User);
    for (auto U : CU->users())
      FindUsingFunctions(U, functions);
  }

  void UpdateFunctionToResourceInfo(const DxilResourceBase *resource,
                                    uint32_t offset) {
    Constant *var = resource->GetGlobalSymbol();
    if (var) {
      for (auto user : var->users()) {
        // Find the function(s).
        llvm::SmallVector<const llvm::Function *, 8> functions;
        FindUsingFunctions(user, functions);
        for (const llvm::Function *F : functions) {
          if (m_FuncToResNameOffset.find(F) == m_FuncToResNameOffset.end()) {
            m_FuncToResNameOffset[F] = Indices();
          }
          m_FuncToResNameOffset[F].insert(offset);
        }
      }
    }
  }

  void InsertToResourceTable(DxilResourceBase &resource,
                             ResourceClass resourceClass,
                             uint32_t &resourceIndex) {
    uint32_t stringIndex = Builder.InsertString(resource.GetGlobalName());
    UpdateFunctionToResourceInfo(&resource, resourceIndex++);
    RDAT::RuntimeDataResourceInfo info = {};
    info.ID = resource.GetID();
    info.Class = static_cast<uint32_t>(resourceClass);
    info.Kind = static_cast<uint32_t>(resource.GetKind());
    info.Space = resource.GetSpaceID();
    info.LowerBound = resource.GetLowerBound();
    info.UpperBound = resource.GetUpperBound();
    info.Name = stringIndex;
    info.Flags = 0;
    if (ResourceClass::UAV == resourceClass) {
      DxilResource *pRes = static_cast<DxilResource *>(&resource);
      if (pRes->HasCounter())
        info.Flags |= static_cast<uint32_t>(RDAT::DxilResourceFlag::UAVCounter);
      if (pRes->IsGloballyCoherent())
        info.Flags |=
            static_cast<uint32_t>(RDAT::DxilResourceFlag::UAVGloballyCoherent);
      if (pRes->IsReorderCoherent())
        info.Flags |=
            static_cast<uint32_t>(RDAT::DxilResourceFlag::UAVReorderCoherent);
      if (pRes->IsROV())
        info.Flags |= static_cast<uint32_t>(
            RDAT::DxilResourceFlag::UAVRasterizerOrderedView);
      if (pRes->HasAtomic64Use())
        info.Flags |=
            static_cast<uint32_t>(RDAT::DxilResourceFlag::Atomics64Use);
      // TODO: add dynamic index flag
    }
    m_pResourceTable->Insert(info);
  }

  void UpdateResourceInfo(const DxilModule &DM) {
    // Try to allocate string table for resources. String table is a sequence
    // of strings delimited by \0
    uint32_t resourceIndex = 0;
    for (auto &resource : DM.GetCBuffers()) {
      InsertToResourceTable(*resource.get(), ResourceClass::CBuffer,
                            resourceIndex);
    }
    for (auto &resource : DM.GetSamplers()) {
      InsertToResourceTable(*resource.get(), ResourceClass::Sampler,
                            resourceIndex);
    }
    for (auto &resource : DM.GetSRVs()) {
      InsertToResourceTable(*resource.get(), ResourceClass::SRV, resourceIndex);
    }
    for (auto &resource : DM.GetUAVs()) {
      InsertToResourceTable(*resource.get(), ResourceClass::UAV, resourceIndex);
    }
  }

  void UpdateFunctionDependency(llvm::Function *F) {
    for (const llvm::User *user : F->users()) {
      llvm::SmallVector<const llvm::Function *, 8> functions;
      FindUsingFunctions(user, functions);
      for (const llvm::Function *userFunction : functions) {
        uint32_t index = Builder.InsertString(F->getName());
        if (m_FuncToDependencies.find(userFunction) ==
            m_FuncToDependencies.end()) {
          m_FuncToDependencies[userFunction] = Indices();
        }
        m_FuncToDependencies[userFunction].insert(index);
      }
    }
  }

  uint32_t AddSigElements(const DxilSignature &sig, uint32_t &shaderFlags,
                          uint8_t *pOutputStreamMask = nullptr) {
    shaderFlags = 0; // Fresh flags each call
    SmallVector<uint32_t, 16> rdatElements;
    for (auto &&E : sig.GetElements()) {
      RDAT::SignatureElement e = {};
      e.SemanticName = Builder.InsertString(E->GetSemanticName());
      e.SemanticIndices = Builder.InsertArray(E->GetSemanticIndexVec().begin(),
                                              E->GetSemanticIndexVec().end());
      e.SemanticKind = (uint8_t)E->GetKind();
      e.ComponentType = (uint8_t)E->GetCompType().GetKind();
      e.InterpolationMode = (uint8_t)E->GetInterpolationMode()->GetKind();
      e.StartRow = E->IsAllocated() ? E->GetStartRow() : 0xFF;
      e.SetCols(E->GetCols());
      e.SetStartCol(E->GetStartCol());
      e.SetOutputStream(E->GetOutputStream());
      e.SetUsageMask(E->GetUsageMask());
      e.SetDynamicIndexMask(E->GetDynIdxCompMask());
      rdatElements.push_back(Builder.InsertRecord(e));

      if (E->GetKind() == DXIL::SemanticKind::Position)
        shaderFlags |= (uint32_t)DxilShaderFlags::OutputPositionPresent;
      if (E->GetInterpolationMode()->IsAnySample() ||
          E->GetKind() == Semantic::Kind::SampleIndex)
        shaderFlags |= (uint32_t)DxilShaderFlags::SampleFrequency;
      if (E->IsAnyDepth())
        shaderFlags |= (uint32_t)DxilShaderFlags::DepthOutput;

      if (pOutputStreamMask)
        *pOutputStreamMask |= 1 << E->GetOutputStream();
    }
    return Builder.InsertArray(rdatElements.begin(), rdatElements.end());
  }

  uint32_t AddIONodes(const std::vector<NodeIOProperties> &nodes) {
    SmallVector<uint32_t, 16> rdatNodes;
    for (auto &N : nodes) {
      RDAT::IONode ioNode = {};
      ioNode.IOFlagsAndKind = N.Flags;
      SmallVector<uint32_t, 16> nodeAttribs;
      RDAT::NodeShaderIOAttrib nAttrib = {};
      if (N.Flags.IsOutputNode()) {
        nAttrib = {};
        nAttrib.AttribKind = (uint32_t)NodeAttribKind::OutputID;
        RDAT::NodeID ID = {};
        ID.Name = Builder.InsertString(N.OutputID.Name);
        ID.Index = N.OutputID.Index;
        nAttrib.OutputID = Builder.InsertRecord(ID);
        nodeAttribs.push_back(Builder.InsertRecord(nAttrib));

        nAttrib = {};
        nAttrib.AttribKind = (uint32_t)NodeAttribKind::OutputArraySize;
        nAttrib.OutputArraySize = N.OutputArraySize;
        nodeAttribs.push_back(Builder.InsertRecord(nAttrib));

        // Only include if these are specified
        if (N.MaxRecords) {
          nAttrib = {};
          nAttrib.AttribKind = (uint32_t)NodeAttribKind::MaxRecords;
          nAttrib.MaxRecords = N.MaxRecords;
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        } else if (N.MaxRecordsSharedWith >= 0) {
          nAttrib = {};
          nAttrib.AttribKind =
              (uint32_t)RDAT::NodeAttribKind::MaxRecordsSharedWith;
          nAttrib.MaxRecordsSharedWith = N.MaxRecordsSharedWith;
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        }
        if (N.AllowSparseNodes) {
          nAttrib = {};
          nAttrib.AttribKind = (uint32_t)RDAT::NodeAttribKind::AllowSparseNodes;
          nAttrib.AllowSparseNodes = N.AllowSparseNodes;
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        }
      } else if (N.Flags.IsInputRecord()) {
        if (N.MaxRecords) {
          nAttrib = {};
          nAttrib.AttribKind = (uint32_t)NodeAttribKind::MaxRecords;
          nAttrib.MaxRecords = N.MaxRecords;
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        }
      }

      // Common attributes
      if (N.RecordType.size) {
        nAttrib = {};
        nAttrib.AttribKind = (uint32_t)NodeAttribKind::RecordSizeInBytes;
        nAttrib.RecordSizeInBytes = N.RecordType.size;
        nodeAttribs.push_back(Builder.InsertRecord(nAttrib));

        if (N.RecordType.SV_DispatchGrid.ComponentType !=
            DXIL::ComponentType::Invalid) {
          nAttrib = {};
          nAttrib.AttribKind = (uint32_t)NodeAttribKind::RecordDispatchGrid;
          nAttrib.RecordDispatchGrid.ByteOffset =
              (uint16_t)N.RecordType.SV_DispatchGrid.ByteOffset;
          nAttrib.RecordDispatchGrid.SetComponentType(
              N.RecordType.SV_DispatchGrid.ComponentType);
          nAttrib.RecordDispatchGrid.SetNumComponents(
              N.RecordType.SV_DispatchGrid.NumComponents);
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        }

        if (N.RecordType.alignment) {
          nAttrib = {};
          nAttrib.AttribKind = (uint32_t)NodeAttribKind::RecordAlignmentInBytes;
          nAttrib.RecordAlignmentInBytes = N.RecordType.alignment;
          nodeAttribs.push_back(Builder.InsertRecord(nAttrib));
        }
      }

      ioNode.Attribs =
          Builder.InsertArray(nodeAttribs.begin(), nodeAttribs.end());
      rdatNodes.push_back(Builder.InsertRecord(ioNode));
    }
    return Builder.InsertArray(rdatNodes.begin(), rdatNodes.end());
  }

  uint32_t AddShaderInfo(llvm::Function &function,
                         const DxilEntryProps &entryProps,
                         RuntimeDataFunctionInfo2 &funcInfo,
                         const ShaderFlags &flags, uint32_t tgsmSizeInBytes) {
    const DxilFunctionProps &props = entryProps.props;
    const DxilEntrySignature &sig = entryProps.sig;
    if (flags.GetViewID())
      funcInfo.ShaderFlags |= (uint16_t)DxilShaderFlags::UsesViewID;
    uint32_t shaderFlags = 0;
    switch (props.shaderKind) {
    case ShaderKind::Pixel: {
      RDAT::PSInfo info = {};
      info.SigInputElements = AddSigElements(sig.InputSignature, shaderFlags);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags & (uint16_t)DxilShaderFlags::SampleFrequency);
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags & (uint16_t)DxilShaderFlags::DepthOutput);
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Vertex: {
      RDAT::VSInfo info = {};
      info.SigInputElements = AddSigElements(sig.InputSignature, shaderFlags);
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags &
                     (uint16_t)DxilShaderFlags::OutputPositionPresent);
      // TODO: Fill in ViewID related masks
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Geometry: {
      RDAT::GSInfo info = {};
      info.SigInputElements = AddSigElements(sig.InputSignature, shaderFlags);
      shaderFlags = 0;
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags,
                                              &info.OutputStreamMask);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags &
                     (uint16_t)DxilShaderFlags::OutputPositionPresent);
      // TODO: Fill in ViewID related masks
      info.InputPrimitive = (uint8_t)props.ShaderProps.GS.inputPrimitive;
      info.OutputTopology =
          (uint8_t)props.ShaderProps.GS.streamPrimitiveTopologies[0];
      info.MaxVertexCount = (uint8_t)props.ShaderProps.GS.maxVertexCount;
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Hull: {
      RDAT::HSInfo info = {};
      info.SigInputElements = AddSigElements(sig.InputSignature, shaderFlags);
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags);
      info.SigPatchConstOutputElements =
          AddSigElements(sig.PatchConstOrPrimSignature, shaderFlags);
      // TODO: Fill in ViewID related masks
      info.InputControlPointCount =
          (uint8_t)props.ShaderProps.HS.inputControlPoints;
      info.OutputControlPointCount =
          (uint8_t)props.ShaderProps.HS.outputControlPoints;
      info.TessellatorDomain = (uint8_t)props.ShaderProps.HS.domain;
      info.TessellatorOutputPrimitive =
          (uint8_t)props.ShaderProps.HS.outputPrimitive;
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Domain: {
      RDAT::DSInfo info = {};
      info.SigInputElements = AddSigElements(sig.InputSignature, shaderFlags);
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags &
                     (uint16_t)DxilShaderFlags::OutputPositionPresent);
      info.SigPatchConstInputElements =
          AddSigElements(sig.PatchConstOrPrimSignature, shaderFlags);
      // TODO: Fill in ViewID related masks
      info.InputControlPointCount =
          (uint8_t)props.ShaderProps.DS.inputControlPoints;
      info.TessellatorDomain = (uint8_t)props.ShaderProps.DS.domain;
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Compute: {
      RDAT::CSInfo info = {};
      info.NumThreads =
          Builder.InsertArray(&props.numThreads[0], &props.numThreads[0] + 3);
      info.GroupSharedBytesUsed = tgsmSizeInBytes;
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Mesh: {
      RDAT::MSInfo info = {};
      info.SigOutputElements = AddSigElements(sig.OutputSignature, shaderFlags);
      funcInfo.ShaderFlags |=
          (uint16_t)(shaderFlags &
                     (uint16_t)DxilShaderFlags::OutputPositionPresent);
      info.SigPrimOutputElements =
          AddSigElements(sig.PatchConstOrPrimSignature, shaderFlags);
      // TODO: Fill in ViewID related masks
      info.NumThreads =
          Builder.InsertArray(&props.numThreads[0], &props.numThreads[0] + 3);
      info.GroupSharedBytesUsed = tgsmSizeInBytes;
      info.GroupSharedBytesDependentOnViewID =
          (uint32_t)0; // TODO: same thing (note: this isn't filled in for PSV!)
      info.PayloadSizeInBytes =
          (uint32_t)props.ShaderProps.MS.payloadSizeInBytes;
      info.MaxOutputVertices = (uint16_t)props.ShaderProps.MS.maxVertexCount;
      info.MaxOutputPrimitives =
          (uint16_t)props.ShaderProps.MS.maxPrimitiveCount;
      info.MeshOutputTopology = (uint8_t)props.ShaderProps.MS.outputTopology;
      return Builder.InsertRecord(info);
    } break;
    case ShaderKind::Amplification: {
      RDAT::ASInfo info = {};
      info.NumThreads =
          Builder.InsertArray(&props.numThreads[0], &props.numThreads[0] + 3);
      info.GroupSharedBytesUsed = tgsmSizeInBytes;
      info.PayloadSizeInBytes =
          (uint32_t)props.ShaderProps.AS.payloadSizeInBytes;
      return Builder.InsertRecord(info);
    } break;
    }
    return RDAT_NULL_REF;
  }

  uint32_t AddShaderNodeInfo(const DxilModule &DM, llvm::Function &function,
                             const DxilEntryProps &entryProps,
                             RuntimeDataFunctionInfo2 &funcInfo,
                             uint32_t tgsmSizeInBytes) {
    const DxilFunctionProps &props = entryProps.props;

    // Add node info
    RDAT::NodeShaderInfo nInfo = {};

    RDAT::NodeShaderFuncAttrib nAttrib = {};
    SmallVector<uint32_t, 16> funcAttribs;

    // LaunchType is technically optional, but less optional
    nInfo.LaunchType = (uint32_t)props.Node.LaunchType;
    nInfo.GroupSharedBytesUsed = tgsmSizeInBytes;

    // Add the function attribute fields
    if (!props.NodeShaderID.Name.empty()) {
      nAttrib = {};
      nAttrib.AttribKind = (uint32_t)RDAT::NodeFuncAttribKind::ID;
      RDAT::NodeID ID = {};
      ID.Name = Builder.InsertString(props.NodeShaderID.Name);
      ID.Index = props.NodeShaderID.Index;
      nAttrib.ID = Builder.InsertRecord(ID);
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (props.Node.IsProgramEntry)
      funcInfo.ShaderFlags |= (uint16_t)DxilShaderFlags::NodeProgramEntry;

    if (props.numThreads[0] || props.numThreads[1] || props.numThreads[2]) {
      nAttrib = {};
      nAttrib.AttribKind = (uint32_t)RDAT::NodeFuncAttribKind::NumThreads;
      nAttrib.NumThreads =
          Builder.InsertArray(&props.numThreads[0], &props.numThreads[0] + 3);
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (props.Node.LocalRootArgumentsTableIndex >= 0) {
      nAttrib = {};
      nAttrib.AttribKind =
          (uint32_t)RDAT::NodeFuncAttribKind::LocalRootArgumentsTableIndex;
      nAttrib.LocalRootArgumentsTableIndex =
          props.Node.LocalRootArgumentsTableIndex;
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (!props.NodeShaderSharedInput.Name.empty()) {
      nAttrib = {};
      nAttrib.AttribKind = (uint32_t)RDAT::NodeFuncAttribKind::ShareInputOf;
      RDAT::NodeID ID = {};
      ID.Name = Builder.InsertString(props.NodeShaderSharedInput.Name);
      ID.Index = props.NodeShaderSharedInput.Index;
      nAttrib.ShareInputOf = Builder.InsertRecord(ID);
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (props.Node.DispatchGrid[0] || props.Node.DispatchGrid[1] ||
        props.Node.DispatchGrid[2]) {
      nAttrib = {};
      nAttrib.AttribKind = (uint32_t)RDAT::NodeFuncAttribKind::DispatchGrid;
      nAttrib.DispatchGrid = Builder.InsertArray(
          &props.Node.DispatchGrid[0], &props.Node.DispatchGrid[0] + 3);
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (props.Node.MaxRecursionDepth) {
      nAttrib = {};
      nAttrib.AttribKind =
          (uint32_t)RDAT::NodeFuncAttribKind::MaxRecursionDepth;
      nAttrib.MaxRecursionDepth = props.Node.MaxRecursionDepth;
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    if (props.Node.MaxDispatchGrid[0] || props.Node.MaxDispatchGrid[1] ||
        props.Node.MaxDispatchGrid[2]) {
      nAttrib = {};
      nAttrib.AttribKind = (uint32_t)RDAT::NodeFuncAttribKind::MaxDispatchGrid;
      nAttrib.MaxDispatchGrid = Builder.InsertArray(
          &props.Node.MaxDispatchGrid[0], &props.Node.MaxDispatchGrid[0] + 3);
      funcAttribs.push_back(Builder.InsertRecord(nAttrib));
    }

    nInfo.Attribs = Builder.InsertArray(funcAttribs.begin(), funcAttribs.end());

    // Add the input and output nodes
    nInfo.Inputs = AddIONodes(props.InputNodes);
    nInfo.Outputs = AddIONodes(props.OutputNodes);

    return Builder.InsertRecord(nInfo);
  }

  void UpdateFunctionInfo(const DxilModule &DM) {
    llvm::Module *M = DM.GetModule();

    for (auto &function : M->getFunctionList()) {
      if (function.isDeclaration() && !function.isIntrinsic() &&
          function.getLinkage() ==
              llvm::GlobalValue::LinkageTypes::ExternalLinkage &&
          !OP::IsDxilOpFunc(&function)) {
        // collect unresolved dependencies per function
        UpdateFunctionDependency(&function);
      }
    }

    // Collect total groupshared memory potentially used by every function
    const DataLayout &DL = M->getDataLayout();
    ValueMap<const Function *, uint32_t> TGSMInFunc;
    // Initialize all function TGSM usage to zero
    for (auto &function : M->getFunctionList()) {
      TGSMInFunc[&function] = 0;
    }
    for (GlobalVariable &GV : M->globals()) {
      if (GV.getType()->getAddressSpace() == DXIL::kTGSMAddrSpace) {
        SmallPtrSet<llvm::Function *, 8> completeFuncs;
        SmallVector<User *, 16> WorkList;
        uint32_t gvSize = DL.getTypeAllocSize(GV.getType()->getElementType());

        WorkList.append(GV.user_begin(), GV.user_end());

        while (!WorkList.empty()) {
          User *U = WorkList.pop_back_val();
          // If const, keep going until we find something we can use
          if (isa<Constant>(U)) {
            WorkList.append(U->user_begin(), U->user_end());
            continue;
          }

          if (Instruction *I = dyn_cast<Instruction>(U)) {
            llvm::Function *F = I->getParent()->getParent();
            if (completeFuncs.insert(F).second) {
              // If function is new, process it and its users
              // Add users to the worklist
              WorkList.append(F->user_begin(), F->user_end());
              // Add groupshared size to function's total
              TGSMInFunc[F] += gvSize;
            }
          }
        }
      }
    }

    for (auto &function : M->getFunctionList()) {
      if (!function.isDeclaration()) {
        StringRef mangled = function.getName();
        StringRef unmangled =
            hlsl::dxilutil::DemangleFunctionName(function.getName());
        uint32_t mangledIndex = Builder.InsertString(mangled);
        uint32_t unmangledIndex = Builder.InsertString(unmangled);
        // Update resource Index
        uint32_t resourceIndex = RDAT_NULL_REF;
        uint32_t functionDependencies = RDAT_NULL_REF;
        uint32_t payloadSizeInBytes = 0;
        uint32_t attrSizeInBytes = 0;
        DXIL::ShaderKind shaderKind = DXIL::ShaderKind::Library;
        uint32_t shaderInfo = RDAT_NULL_REF;

        if (m_FuncToResNameOffset.find(&function) !=
            m_FuncToResNameOffset.end())
          resourceIndex =
              Builder.InsertArray(m_FuncToResNameOffset[&function].begin(),
                                  m_FuncToResNameOffset[&function].end());
        if (m_FuncToDependencies.find(&function) != m_FuncToDependencies.end())
          functionDependencies =
              Builder.InsertArray(m_FuncToDependencies[&function].begin(),
                                  m_FuncToDependencies[&function].end());
        RuntimeDataFunctionInfo2 info_latest = {};
        RuntimeDataFunctionInfo &info = info_latest;
        RuntimeDataFunctionInfo2 *pInfo2 = (sizeof(RuntimeDataFunctionInfo2) <=
                                            m_pFunctionTable->GetRecordStride())
                                               ? &info_latest
                                               : nullptr;

        const DxilModule::ShaderCompatInfo &compatInfo =
            *DM.GetCompatInfoForFunction(&function);

        if (DM.HasDxilFunctionProps(&function)) {
          const DxilFunctionProps &props = DM.GetDxilFunctionProps(&function);
          if (props.IsClosestHit() || props.IsAnyHit()) {
            payloadSizeInBytes = props.ShaderProps.Ray.payloadSizeInBytes;
            attrSizeInBytes = props.ShaderProps.Ray.attributeSizeInBytes;
          } else if (props.IsMiss()) {
            payloadSizeInBytes = props.ShaderProps.Ray.payloadSizeInBytes;
          } else if (props.IsCallable()) {
            payloadSizeInBytes = props.ShaderProps.Ray.paramSizeInBytes;
          }
          shaderKind = props.shaderKind;
          DxilWaveSize waveSize = props.WaveSize;
          if (pInfo2 && DM.HasDxilEntryProps(&function)) {
            const auto &entryProps = DM.GetDxilEntryProps(&function);
            if (waveSize.IsDefined()) {
              pInfo2->MinimumExpectedWaveLaneCount = (uint8_t)waveSize.Min;
              pInfo2->MaximumExpectedWaveLaneCount =
                  (waveSize.IsRange()) ? (uint8_t)waveSize.Max
                                       : (uint8_t)waveSize.Min;
            }
            pInfo2->ShaderFlags = 0;
            if (entryProps.props.IsNode()) {
              shaderInfo = AddShaderNodeInfo(DM, function, entryProps, *pInfo2,
                                             TGSMInFunc[&function]);
            } else if (DXIL::CompareVersions(m_ValMajor, m_ValMinor, 1, 8) >
                       0) {
              shaderInfo =
                  AddShaderInfo(function, entryProps, *pInfo2,
                                compatInfo.shaderFlags, TGSMInFunc[&function]);
            }
          }
        }
        info.Name = mangledIndex;
        info.UnmangledName = unmangledIndex;
        info.ShaderKind = static_cast<uint32_t>(shaderKind);
        if (pInfo2)
          pInfo2->RawShaderRef = shaderInfo;
        info.Resources = resourceIndex;
        info.FunctionDependencies = functionDependencies;
        info.PayloadSizeInBytes = payloadSizeInBytes;
        info.AttributeSizeInBytes = attrSizeInBytes;
        info.SetFeatureFlags(compatInfo.shaderFlags.GetFeatureInfo());
        info.ShaderStageFlag = compatInfo.mask;
        info.MinShaderTarget =
            EncodeVersion((DXIL::ShaderKind)shaderKind, compatInfo.minMajor,
                          compatInfo.minMinor);
        m_pFunctionTable->Insert(info_latest);
      }
    }
  }

  void UpdateSubobjectInfo(const DxilModule &DM) {
    if (!DM.GetSubobjects())
      return;
    for (auto &it : DM.GetSubobjects()->GetSubobjects()) {
      auto &obj = *it.second;
      RuntimeDataSubobjectInfo info = {};
      info.Name = Builder.InsertString(obj.GetName());
      info.Kind = (uint32_t)obj.GetKind();
      bool bLocalRS = false;
      switch (obj.GetKind()) {
      case DXIL::SubobjectKind::StateObjectConfig:
        obj.GetStateObjectConfig(info.StateObjectConfig.Flags);
        break;
      case DXIL::SubobjectKind::LocalRootSignature:
        bLocalRS = true;
        LLVM_FALLTHROUGH;
      case DXIL::SubobjectKind::GlobalRootSignature: {
        const void *Data;
        obj.GetRootSignature(bLocalRS, Data, info.RootSignature.Data.Size);
        info.RootSignature.Data.Offset = Builder.GetRawBytesPart().Insert(
            Data, info.RootSignature.Data.Size);
        break;
      }
      case DXIL::SubobjectKind::SubobjectToExportsAssociation: {
        llvm::StringRef Subobject;
        const char *const *Exports;
        uint32_t NumExports;
        std::vector<uint32_t> ExportIndices;
        obj.GetSubobjectToExportsAssociation(Subobject, Exports, NumExports);
        info.SubobjectToExportsAssociation.Subobject =
            Builder.InsertString(Subobject);
        ExportIndices.resize(NumExports);
        for (unsigned i = 0; i < NumExports; ++i) {
          ExportIndices[i] = Builder.InsertString(Exports[i]);
        }
        info.SubobjectToExportsAssociation.Exports =
            Builder.InsertArray(ExportIndices.begin(), ExportIndices.end());
        break;
      }
      case DXIL::SubobjectKind::RaytracingShaderConfig:
        obj.GetRaytracingShaderConfig(
            info.RaytracingShaderConfig.MaxPayloadSizeInBytes,
            info.RaytracingShaderConfig.MaxAttributeSizeInBytes);
        break;
      case DXIL::SubobjectKind::RaytracingPipelineConfig:
        obj.GetRaytracingPipelineConfig(
            info.RaytracingPipelineConfig.MaxTraceRecursionDepth);
        break;
      case DXIL::SubobjectKind::HitGroup: {
        HitGroupType hgType;
        StringRef AnyHit;
        StringRef ClosestHit;
        StringRef Intersection;
        obj.GetHitGroup(hgType, AnyHit, ClosestHit, Intersection);
        info.HitGroup.Type = (uint32_t)hgType;
        info.HitGroup.AnyHit = Builder.InsertString(AnyHit);
        info.HitGroup.ClosestHit = Builder.InsertString(ClosestHit);
        info.HitGroup.Intersection = Builder.InsertString(Intersection);
        break;
      }
      case DXIL::SubobjectKind::RaytracingPipelineConfig1:
        obj.GetRaytracingPipelineConfig1(
            info.RaytracingPipelineConfig1.MaxTraceRecursionDepth,
            info.RaytracingPipelineConfig1.Flags);
        break;
      }
      m_pSubobjectTable->Insert(info);
    }
  }

  static bool GetRecordDuplicationAllowed(const DxilModule &mod) {
    unsigned valMajor, valMinor;
    mod.GetValidatorVersion(valMajor, valMinor);
    const bool bRecordDeduplicationEnabled =
        DXIL::CompareVersions(valMajor, valMinor, 1, 7) >= 0;
    return bRecordDeduplicationEnabled;
  }

public:
  DxilRDATWriter(DxilModule &mod) : Builder(GetRecordDuplicationAllowed(mod)) {
    // Keep track of validator version so we can make a compatible RDAT
    mod.GetValidatorVersion(m_ValMajor, m_ValMinor);
    RDAT::RuntimeDataPartType maxAllowedType =
        RDAT::MaxPartTypeForValVer(m_ValMajor, m_ValMinor);

    mod.ComputeShaderCompatInfo();

    // Instantiate the parts in the order that validator expects.
    Builder.GetStringBufferPart();
    m_pResourceTable = Builder.GetOrAddTable<RDAT::RuntimeDataResourceInfo>();
    m_pFunctionTable = Builder.GetOrAddTable<RuntimeDataFunctionInfo>();
    if (DXIL::CompareVersions(m_ValMajor, m_ValMinor, 1, 8) >= 0) {
      m_pFunctionTable->SetRecordStride(sizeof(RuntimeDataFunctionInfo2));
    } else {
      m_pFunctionTable->SetRecordStride(sizeof(RuntimeDataFunctionInfo));
    }
    Builder.GetIndexArraysPart();
    Builder.GetRawBytesPart();
    if (RDAT::RecordTraits<RuntimeDataSubobjectInfo>::PartType() <=
        maxAllowedType)
      m_pSubobjectTable = Builder.GetOrAddTable<RuntimeDataSubobjectInfo>();

// Once per table.
#define RDAT_STRUCT_TABLE(type, table)                                         \
  if (RDAT::RecordTraits<RDAT::type>::PartType() <= maxAllowedType)            \
    (void)Builder.GetOrAddTable<RDAT::type>();

#define DEF_RDAT_TYPES DEF_RDAT_DEFAULTS
#include "dxc/DxilContainer/RDAT_Macros.inl"

    UpdateResourceInfo(mod);
    UpdateFunctionInfo(mod);
    if (m_pSubobjectTable)
      UpdateSubobjectInfo(mod);
  }

  uint32_t size() const override { return Builder.size(); }

  void write(AbstractMemoryStream *pStream) override {
    StringRef data = Builder.FinalizeAndGetData();
    ULONG uWritten = 0;
    IFT(pStream->Write(data.data(), data.size(), &uWritten));
  }
};

DxilPartWriter *hlsl::NewPSVWriter(const DxilModule &M, uint32_t PSVVersion) {
  return new DxilPSVWriter(M, PSVVersion);
}

DxilPartWriter *hlsl::NewRDATWriter(DxilModule &M) {
  return new DxilRDATWriter(M);
}

DxilPartWriter *hlsl::NewVersionWriter(IDxcVersionInfo *DXCVersionInfo) {
  return new DxilVersionWriter(DXCVersionInfo);
}

class DxilContainerWriter_impl : public DxilContainerWriter {
private:
  class DxilPart {
  public:
    DxilPartHeader Header;
    WriteFn Write;
    DxilPart(uint32_t fourCC, uint32_t size, WriteFn write) : Write(write) {
      Header.PartFourCC = fourCC;
      Header.PartSize = size;
    }
  };

  llvm::SmallVector<DxilPart, 8> m_Parts;
  bool m_bUnaligned;
  bool m_bHasPrivateData;

public:
  DxilContainerWriter_impl(bool bUnaligned)
      : m_bUnaligned(bUnaligned), m_bHasPrivateData(false) {}

  void AddPart(uint32_t FourCC, uint32_t Size, WriteFn Write) override {
    // Alignment required for all parts except private data, which must be last.
    IFTBOOL(!m_bHasPrivateData &&
                "private data must be last, and cannot be added twice.",
            DXC_E_CONTAINER_INVALID);
    if (FourCC == DFCC_PrivateData) {
      m_bHasPrivateData = true;
    } else if (!m_bUnaligned) {
      IFTBOOL((Size % sizeof(uint32_t)) == 0, DXC_E_CONTAINER_INVALID);
    }
    m_Parts.emplace_back(FourCC, Size, Write);
  }

  uint32_t size() const override {
    uint32_t partSize = 0;
    for (auto &part : m_Parts) {
      partSize += part.Header.PartSize;
    }
    return (uint32_t)GetDxilContainerSizeFromParts((uint32_t)m_Parts.size(),
                                                   partSize);
  }

  void write(AbstractMemoryStream *pStream) override {
    DxilContainerHeader header;
    const uint32_t PartCount = (uint32_t)m_Parts.size();
    uint32_t containerSizeInBytes = size();
    InitDxilContainer(&header, PartCount, containerSizeInBytes);
    IFT(pStream->Reserve(header.ContainerSizeInBytes));
    IFT(WriteStreamValue(pStream, header));
    uint32_t offset = sizeof(header) + (uint32_t)GetOffsetTableSize(PartCount);
    for (auto &&part : m_Parts) {
      IFT(WriteStreamValue(pStream, offset));
      offset += sizeof(DxilPartHeader) + part.Header.PartSize;
    }
    for (auto &&part : m_Parts) {
      IFT(WriteStreamValue(pStream, part.Header));
      size_t start = pStream->GetPosition();
      part.Write(pStream);
      DXASSERT_LOCALVAR(
          start, pStream->GetPosition() - start == (size_t)part.Header.PartSize,
          "out of bound");
    }
    DXASSERT(containerSizeInBytes == (uint32_t)pStream->GetPosition(),
             "else stream size is incorrect");
  }
};

DxilContainerWriter *hlsl::NewDxilContainerWriter(bool bUnaligned) {
  return new DxilContainerWriter_impl(bUnaligned);
}

static bool HasDebugInfoOrLineNumbers(const Module &M) {
  return llvm::getDebugMetadataVersionFromModule(M) != 0 ||
         llvm::hasDebugInfo(M);
}

static void GetPaddedProgramPartSize(AbstractMemoryStream *pStream,
                                     uint32_t &bitcodeInUInt32,
                                     uint32_t &bitcodePaddingBytes) {
  bitcodeInUInt32 = pStream->GetPtrSize();
  bitcodePaddingBytes = (bitcodeInUInt32 % 4);
  bitcodeInUInt32 = (bitcodeInUInt32 / 4) + (bitcodePaddingBytes ? 1 : 0);
}

void hlsl::WriteProgramPart(const ShaderModel *pModel,
                            AbstractMemoryStream *pModuleBitcode,
                            IStream *pStream) {
  DXASSERT(pModel != nullptr, "else generation should have failed");
  DxilProgramHeader programHeader;
  uint32_t shaderVersion =
      EncodeVersion(pModel->GetKind(), pModel->GetMajor(), pModel->GetMinor());
  unsigned dxilMajor, dxilMinor;
  pModel->GetDxilVersion(dxilMajor, dxilMinor);
  uint32_t dxilVersion = DXIL::MakeDxilVersion(dxilMajor, dxilMinor);
  InitProgramHeader(programHeader, shaderVersion, dxilVersion,
                    pModuleBitcode->GetPtrSize());

  uint32_t programInUInt32, programPaddingBytes;
  GetPaddedProgramPartSize(pModuleBitcode, programInUInt32,
                           programPaddingBytes);

  ULONG cbWritten;
  IFT(WriteStreamValue(pStream, programHeader));
  IFT(pStream->Write(pModuleBitcode->GetPtr(), pModuleBitcode->GetPtrSize(),
                     &cbWritten));
  if (programPaddingBytes) {
    uint32_t paddingValue = 0;
    IFT(pStream->Write(&paddingValue, programPaddingBytes, &cbWritten));
  }
}

namespace {

class RootSignatureWriter : public DxilPartWriter {
private:
  std::vector<uint8_t> m_Sig;

public:
  RootSignatureWriter(std::vector<uint8_t> &&S) : m_Sig(std::move(S)) {}
  uint32_t size() const { return m_Sig.size(); }
  void write(AbstractMemoryStream *pStream) {
    ULONG cbWritten;
    IFT(pStream->Write(m_Sig.data(), size(), &cbWritten));
  }
};

} // namespace

void hlsl::ReEmitLatestReflectionData(llvm::Module *pM) {
  // Retain usage information in metadata for reflection by:
  // Upgrade validator version, re-emit metadata
  // 0,0 = Not meant to be validated, support latest

  DxilModule &DM = pM->GetOrCreateDxilModule();

  DM.SetValidatorVersion(0, 0);
  DM.ReEmitDxilResources();
  DM.EmitDxilCounters();
}

static std::unique_ptr<Module> CloneModuleForReflection(Module *pM) {
  DxilModule &DM = pM->GetOrCreateDxilModule();

  unsigned ValMajor = 0, ValMinor = 0;
  DM.GetValidatorVersion(ValMajor, ValMinor);

  // Emit the latest reflection metadata
  hlsl::ReEmitLatestReflectionData(pM);

  // Clone module
  std::unique_ptr<Module> reflectionModule(llvm::CloneModule(pM));

  // Now restore validator version on main module and re-emit metadata.
  DM.SetValidatorVersion(ValMajor, ValMinor);
  DM.ReEmitDxilResources();

  return reflectionModule;
}

void hlsl::StripAndCreateReflectionStream(
    Module *pReflectionM, uint32_t *pReflectionPartSizeInBytes,
    AbstractMemoryStream **ppReflectionStreamOut) {
  for (Function &F : pReflectionM->functions()) {
    if (!F.isDeclaration()) {
      F.deleteBody();
    }
  }

  uint32_t reflectPartSizeInBytes = 0;
  CComPtr<AbstractMemoryStream> pReflectionBitcodeStream;

  IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pReflectionBitcodeStream));
  raw_stream_ostream outStream(pReflectionBitcodeStream.p);
  WriteBitcodeToFile(pReflectionM, outStream, false);
  outStream.flush();
  uint32_t reflectInUInt32 = 0, reflectPaddingBytes = 0;
  GetPaddedProgramPartSize(pReflectionBitcodeStream, reflectInUInt32,
                           reflectPaddingBytes);
  reflectPartSizeInBytes =
      reflectInUInt32 * sizeof(uint32_t) + sizeof(DxilProgramHeader);

  *pReflectionPartSizeInBytes = reflectPartSizeInBytes;
  *ppReflectionStreamOut = pReflectionBitcodeStream.Detach();
}

void hlsl::SerializeDxilContainerForModule(
    DxilModule *pModule, AbstractMemoryStream *pModuleBitcode,
    IDxcVersionInfo *DXCVersionInfo, AbstractMemoryStream *pFinalStream,
    llvm::StringRef DebugName, SerializeDxilFlags Flags,
    DxilShaderHash *pShaderHashOut, AbstractMemoryStream *pReflectionStreamOut,
    AbstractMemoryStream *pRootSigStreamOut, void *pPrivateData,
    size_t PrivateDataSize) {
  llvm::TimeTraceScope TimeScope("SerializeDxilContainer", StringRef(""));
  // TODO: add a flag to update the module and remove information that is not
  // part of DXIL proper and is used only to assemble the container.

  DXASSERT_NOMSG(pModule != nullptr);
  DXASSERT_NOMSG(pModuleBitcode != nullptr);
  DXASSERT_NOMSG(pFinalStream != nullptr);

  unsigned ValMajor, ValMinor;
  pModule->GetValidatorVersion(ValMajor, ValMinor);
  bool bValidatorAtLeast_1_8 =
      DXIL::CompareVersions(ValMajor, ValMinor, 1, 8) >= 0;
  if (DXIL::CompareVersions(ValMajor, ValMinor, 1, 1) < 0)
    Flags &= ~SerializeDxilFlags::IncludeDebugNamePart;
  bool bSupportsShaderHash =
      DXIL::CompareVersions(ValMajor, ValMinor, 1, 5) >= 0;
  bool bCompat_1_4 = DXIL::CompareVersions(ValMajor, ValMinor, 1, 5) < 0;
  bool bUnaligned = DXIL::CompareVersions(ValMajor, ValMinor, 1, 7) < 0;
  bool bEmitReflection =
      Flags & SerializeDxilFlags::IncludeReflectionPart || pReflectionStreamOut;

  DxilContainerWriter_impl writer(bUnaligned);

  // Write the feature part.
  DxilFeatureInfoWriter featureInfoWriter(*pModule);
  writer.AddPart(
      DFCC_FeatureInfo, featureInfoWriter.size(),
      [&](AbstractMemoryStream *pStream) { featureInfoWriter.write(pStream); });

  std::unique_ptr<DxilProgramSignatureWriter> pInputSigWriter = nullptr;
  std::unique_ptr<DxilProgramSignatureWriter> pOutputSigWriter = nullptr;
  std::unique_ptr<DxilProgramSignatureWriter> pPatchConstOrPrimSigWriter =
      nullptr;
  if (!pModule->GetShaderModel()->IsLib()) {
    DXIL::TessellatorDomain domain = DXIL::TessellatorDomain::Undefined;
    if (pModule->GetShaderModel()->IsHS() || pModule->GetShaderModel()->IsDS())
      domain = pModule->GetTessellatorDomain();
    pInputSigWriter = llvm::make_unique<DxilProgramSignatureWriter>(
        pModule->GetInputSignature(), domain,
        /*IsInput*/ true,
        /*UseMinPrecision*/ pModule->GetUseMinPrecision(), bCompat_1_4,
        bUnaligned);
    pOutputSigWriter = llvm::make_unique<DxilProgramSignatureWriter>(
        pModule->GetOutputSignature(), domain,
        /*IsInput*/ false,
        /*UseMinPrecision*/ pModule->GetUseMinPrecision(), bCompat_1_4,
        bUnaligned);
    // Write the input and output signature parts.
    writer.AddPart(DFCC_InputSignature, pInputSigWriter->size(),
                   [&](AbstractMemoryStream *pStream) {
                     pInputSigWriter->write(pStream);
                   });
    writer.AddPart(DFCC_OutputSignature, pOutputSigWriter->size(),
                   [&](AbstractMemoryStream *pStream) {
                     pOutputSigWriter->write(pStream);
                   });

    pPatchConstOrPrimSigWriter = llvm::make_unique<DxilProgramSignatureWriter>(
        pModule->GetPatchConstOrPrimSignature(), domain,
        /*IsInput*/ pModule->GetShaderModel()->IsDS(),
        /*UseMinPrecision*/ pModule->GetUseMinPrecision(), bCompat_1_4,
        bUnaligned);
    if (pModule->GetPatchConstOrPrimSignature().GetElements().size()) {
      writer.AddPart(DFCC_PatchConstantSignature,
                     pPatchConstOrPrimSigWriter->size(),
                     [&](AbstractMemoryStream *pStream) {
                       pPatchConstOrPrimSigWriter->write(pStream);
                     });
    }
  }

  std::unique_ptr<DxilVersionWriter> pVERSWriter = nullptr;
  std::unique_ptr<DxilRDATWriter> pRDATWriter = nullptr;
  std::unique_ptr<DxilPSVWriter> pPSVWriter = nullptr;

  unsigned int major, minor;
  pModule->GetDxilVersion(major, minor);
  RootSignatureWriter rootSigWriter(
      std::move(pModule->GetSerializedRootSignature())); // Grab RS here
  DXASSERT_NOMSG(pModule->GetSerializedRootSignature().empty());

  bool bMetadataStripped = false;
  const hlsl::ShaderModel *pSM = pModule->GetShaderModel();
  if (pSM->IsLib()) {
    DXASSERT(
        pModule->GetSerializedRootSignature().empty(),
        "otherwise, library has root signature outside subobject definitions");
    // Write the DxilCompilerVersion (VERS) part.
    if (DXCVersionInfo && bValidatorAtLeast_1_8) {

      pVERSWriter = llvm::make_unique<DxilVersionWriter>(DXCVersionInfo);

      writer.AddPart(hlsl::DFCC_CompilerVersion, pVERSWriter->size(),
                     [&pVERSWriter](AbstractMemoryStream *pStream) {
                       pVERSWriter->write(pStream);
                       return S_OK;
                     });
    }

    // Write the DxilRuntimeData (RDAT) part.
    pRDATWriter = llvm::make_unique<DxilRDATWriter>(*pModule);
    writer.AddPart(
        DFCC_RuntimeData, pRDATWriter->size(),
        [&](AbstractMemoryStream *pStream) { pRDATWriter->write(pStream); });
    bMetadataStripped |= pModule->StripSubobjectsFromMetadata();
    pModule->ResetSubobjects(nullptr);
  } else {
    // Write the DxilPipelineStateValidation (PSV0) part.
    pPSVWriter = llvm::make_unique<DxilPSVWriter>(*pModule);
    writer.AddPart(
        DFCC_PipelineStateValidation, pPSVWriter->size(),
        [&](AbstractMemoryStream *pStream) { pPSVWriter->write(pStream); });

    // Write the root signature (RTS0) part.
    if (rootSigWriter.size()) {
      if (pRootSigStreamOut) {
        // Write root signature wrapped in container for separate output
        // Root signature container should never be unaligned.
        DxilContainerWriter_impl rootSigContainerWriter(false);
        rootSigContainerWriter.AddPart(DFCC_RootSignature, rootSigWriter.size(),
                                       [&](AbstractMemoryStream *pStream) {
                                         rootSigWriter.write(pStream);
                                       });
        rootSigContainerWriter.write(pRootSigStreamOut);
      }
      if ((Flags & SerializeDxilFlags::StripRootSignature) == 0) {
        // Write embedded root signature
        writer.AddPart(DFCC_RootSignature, rootSigWriter.size(),
                       [&](AbstractMemoryStream *pStream) {
                         rootSigWriter.write(pStream);
                       });
      }
      bMetadataStripped |= pModule->StripRootSignatureFromMetadata();
    }
  }

  // If metadata was stripped, re-serialize the input module.
  CComPtr<AbstractMemoryStream> pInputProgramStream = pModuleBitcode;
  if (bMetadataStripped) {
    pInputProgramStream.Release();
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pInputProgramStream));
    raw_stream_ostream outStream(pInputProgramStream.p);
    WriteBitcodeToFile(pModule->GetModule(), outStream, true);
  }

  // If we have debug information present, serialize it to a debug part, then
  // use the stripped version as the canonical program version.
  CComPtr<AbstractMemoryStream> pProgramStream = pInputProgramStream;
  bool bModuleStripped = false;
  if (HasDebugInfoOrLineNumbers(*pModule->GetModule())) {
    uint32_t debugInUInt32, debugPaddingBytes;
    GetPaddedProgramPartSize(pInputProgramStream, debugInUInt32,
                             debugPaddingBytes);
    if (Flags & SerializeDxilFlags::IncludeDebugInfoPart) {
      writer.AddPart(DFCC_ShaderDebugInfoDXIL,
                     debugInUInt32 * sizeof(uint32_t) +
                         sizeof(DxilProgramHeader),
                     [&](AbstractMemoryStream *pStream) {
                       hlsl::WriteProgramPart(pModule->GetShaderModel(),
                                              pInputProgramStream, pStream);
                     });
    }

    llvm::StripDebugInfo(*pModule->GetModule());
    pModule->StripDebugRelatedCode();
    bModuleStripped = true;
  } else {
    // If no debug info, clear DebugNameDependOnSource
    // (it's default, and this scenario can happen)
    Flags &= ~SerializeDxilFlags::DebugNameDependOnSource;
  }

  uint32_t reflectPartSizeInBytes = 0;
  CComPtr<AbstractMemoryStream> pReflectionBitcodeStream;

  if (bEmitReflection) {
    // Clone module for reflection
    std::unique_ptr<Module> reflectionModule =
        CloneModuleForReflection(pModule->GetModule());
    hlsl::StripAndCreateReflectionStream(reflectionModule.get(),
                                         &reflectPartSizeInBytes,
                                         &pReflectionBitcodeStream);
  }

  if (pReflectionStreamOut) {
    DxilPartHeader partSTAT;
    partSTAT.PartFourCC = DFCC_ShaderStatistics;
    partSTAT.PartSize = reflectPartSizeInBytes;
    IFT(WriteStreamValue(pReflectionStreamOut, partSTAT));
    WriteProgramPart(pModule->GetShaderModel(), pReflectionBitcodeStream,
                     pReflectionStreamOut);

    // If library, we need RDAT part as well.  For now, we just append it
    if (pModule->GetShaderModel()->IsLib()) {
      DxilPartHeader partRDAT;
      partRDAT.PartFourCC = DFCC_RuntimeData;
      partRDAT.PartSize = pRDATWriter->size();
      IFT(WriteStreamValue(pReflectionStreamOut, partRDAT));
      pRDATWriter->write(pReflectionStreamOut);
    }
  }

  if (Flags & SerializeDxilFlags::IncludeReflectionPart) {
    writer.AddPart(
        DFCC_ShaderStatistics, reflectPartSizeInBytes,
        [pModule, pReflectionBitcodeStream](AbstractMemoryStream *pStream) {
          WriteProgramPart(pModule->GetShaderModel(), pReflectionBitcodeStream,
                           pStream);
        });
  }

  if (Flags & SerializeDxilFlags::StripReflectionFromDxilPart) {
    bModuleStripped |= pModule->StripReflection();
  }

  // If debug info or reflection was stripped, re-serialize the module.
  if (bModuleStripped) {
    pProgramStream.Release();
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pProgramStream));
    raw_stream_ostream outStream(pProgramStream.p);
    WriteBitcodeToFile(pModule->GetModule(), outStream, false);
  }

  // Compute hash if needed.
  DxilShaderHash HashContent;
  SmallString<32> HashStr;
  if (bSupportsShaderHash || pShaderHashOut ||
      (Flags & SerializeDxilFlags::IncludeDebugNamePart && DebugName.empty())) {
    // If the debug name should be specific to the sources, base the name on the
    // debug bitcode, which will include the source references, line numbers,
    // etc. Otherwise, do it exclusively on the target shader bitcode.
    llvm::MD5 md5;
    if (Flags & SerializeDxilFlags::DebugNameDependOnSource) {
      md5.update(ArrayRef<uint8_t>(pModuleBitcode->GetPtr(),
                                   pModuleBitcode->GetPtrSize()));
      HashContent.Flags = (uint32_t)DxilShaderHashFlags::IncludesSource;
    } else {
      md5.update(ArrayRef<uint8_t>(pProgramStream->GetPtr(),
                                   pProgramStream->GetPtrSize()));
      HashContent.Flags = (uint32_t)DxilShaderHashFlags::None;
    }
    md5.final(HashContent.Digest);
    md5.stringifyResult(HashContent.Digest, HashStr);
  }

  // Serialize debug name if requested.
  std::string DebugNameStr; // Used if constructing name based on hash
  if (Flags & SerializeDxilFlags::IncludeDebugNamePart) {
    if (DebugName.empty()) {
      DebugNameStr += HashStr;
      DebugNameStr += ".pdb";
      DebugName = DebugNameStr;
    }

    // Calculate the size of the blob part.
    const uint32_t DebugInfoContentLen = PSVALIGN4(
        sizeof(DxilShaderDebugName) + DebugName.size() + 1); // 1 for null

    writer.AddPart(
        DFCC_ShaderDebugName, DebugInfoContentLen,
        [DebugName](AbstractMemoryStream *pStream) {
          DxilShaderDebugName NameContent;
          NameContent.Flags = 0;
          NameContent.NameLength = DebugName.size();
          IFT(WriteStreamValue(pStream, NameContent));

          ULONG cbWritten;
          IFT(pStream->Write(DebugName.begin(), DebugName.size(), &cbWritten));
          const char Pad[] = {'\0', '\0', '\0', '\0'};
          // Always writes at least one null to align size
          unsigned padLen =
              (4 - ((sizeof(DxilShaderDebugName) + cbWritten) & 0x3));
          IFT(pStream->Write(Pad, padLen, &cbWritten));
        });
  }

  // Add hash to container if supported by validator version.
  if (bSupportsShaderHash) {
    writer.AddPart(DFCC_ShaderHash, sizeof(HashContent),
                   [HashContent](AbstractMemoryStream *pStream) {
                     IFT(WriteStreamValue(pStream, HashContent));
                   });
  }

  // Write hash to separate output if requested.
  if (pShaderHashOut) {
    memcpy(pShaderHashOut, &HashContent, sizeof(DxilShaderHash));
  }

  // Compute padded bitcode size.
  uint32_t programInUInt32, programPaddingBytes;
  GetPaddedProgramPartSize(pProgramStream, programInUInt32,
                           programPaddingBytes);

  // Write the program part.
  writer.AddPart(
      DFCC_DXIL, programInUInt32 * sizeof(uint32_t) + sizeof(DxilProgramHeader),
      [&](AbstractMemoryStream *pStream) {
        WriteProgramPart(pModule->GetShaderModel(), pProgramStream, pStream);
      });

  // Private data part should be added last when assembling the container
  // becasue there is no garuntee of aligned size
  if (pPrivateData) {
    writer.AddPart(
        hlsl::DFCC_PrivateData, PrivateDataSize,
        [&](AbstractMemoryStream *pStream) {
          ULONG cbWritten;
          IFT(pStream->Write(pPrivateData, PrivateDataSize, &cbWritten));
        });
  }

  writer.write(pFinalStream);
}

void hlsl::SerializeDxilContainerForRootSignature(
    hlsl::RootSignatureHandle *pRootSigHandle,
    AbstractMemoryStream *pFinalStream) {
  DXASSERT_NOMSG(pRootSigHandle != nullptr);
  DXASSERT_NOMSG(pFinalStream != nullptr);
  // Root signature container should never be unaligned.
  DxilContainerWriter_impl writer(false);
  // Write the root signature (RTS0) part.
  DxilProgramRootSignatureWriter rootSigWriter(*pRootSigHandle);
  if (!pRootSigHandle->IsEmpty()) {
    writer.AddPart(
        DFCC_RootSignature, rootSigWriter.size(),
        [&](AbstractMemoryStream *pStream) { rootSigWriter.write(pStream); });
  }
  writer.write(pFinalStream);
}
