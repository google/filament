///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilContainerValidation.cpp                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file provides support for validating DXIL container.                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilContainerAssembler.h"
#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/DxilValidation/DxilValidation.h"

#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilUtil.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include "DxilValidationUtils.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

using namespace llvm;
using namespace hlsl;

using std::unique_ptr;
using std::unordered_set;
using std::vector;

namespace {

// Utility class for setting and restoring the diagnostic context so we may
// capture errors/warnings
struct DiagRestore {
  LLVMContext *Ctx = nullptr;
  void *OrigDiagContext;
  LLVMContext::DiagnosticHandlerTy OrigHandler;

  DiagRestore(llvm::LLVMContext &InputCtx, void *DiagContext) : Ctx(&InputCtx) {
    init(DiagContext);
  }
  DiagRestore(Module *M, void *DiagContext) {
    if (!M)
      return;
    Ctx = &M->getContext();
    init(DiagContext);
  }
  ~DiagRestore() {
    if (!Ctx)
      return;
    Ctx->setDiagnosticHandler(OrigHandler, OrigDiagContext);
  }

private:
  void init(void *DiagContext) {
    OrigHandler = Ctx->getDiagnosticHandler();
    OrigDiagContext = Ctx->getDiagnosticContext();
    Ctx->setDiagnosticHandler(
        hlsl::PrintDiagnosticContext::PrintDiagnosticHandler, DiagContext);
  }
};

static void emitDxilDiag(LLVMContext &Ctx, const char *str) {
  hlsl::dxilutil::EmitErrorOnContext(Ctx, str);
}

class StringTableVerifier {
  std::unordered_map<unsigned, unsigned> OffsetToUseCountMap;
  const PSVStringTable &Table;

public:
  StringTableVerifier(const PSVStringTable &Table) : Table(Table) {
    unsigned Start = 0;
    for (unsigned i = 0; i < Table.Size; ++i) {
      char ch = Table.Table[i];
      if (ch == '\0') {
        OffsetToUseCountMap[Start] = 0;
        Start = i + 1;
      }
    }
    if (Table.Size >= 4) {
      // Remove the '\0's at the end of the table added for padding.
      for (unsigned i = Table.Size - 1; i > Table.Size - 4; --i) {
        if (Table.Table[i] != '\0')
          break;
        OffsetToUseCountMap.erase(i);
      }
    }
  }
  bool MarkUse(unsigned Offset) {
    auto it = OffsetToUseCountMap.find(Offset);
    if (it != OffsetToUseCountMap.end())
      it->second++;
    return Offset < Table.Size;
  }
  void Verify(ValidationContext &ValCtx) {
    for (auto [Offset, UseCount] : OffsetToUseCountMap) {
      if (UseCount != 0)
        continue;
      // DXC will always add a null-terminated string at the beginning of the
      // StringTable. It is OK if it is not used.
      if (Offset == 0 && Table.Table[0] == '\0')
        continue;

      ValCtx.EmitFormatError(ValidationRule::ContainerUnusedItemInTable,
                             {"StringTable", Table.Get(Offset)});
    }
  }
};

class SemanticIndexTableVerifier {
  const PSVSemanticIndexTable &Table;
  llvm::BitVector UseMask;

public:
  SemanticIndexTableVerifier(const PSVSemanticIndexTable &Table)
      : Table(Table), UseMask(Table.Entries, false) {}
  bool MarkUse(unsigned Offset, unsigned Size) {
    if (Table.Table == nullptr)
      return false;
    if (Offset > Table.Entries)
      return false;
    if ((Offset + Size) > Table.Entries)
      return false;
    for (unsigned i = Offset; i < (Offset + Size); ++i) {
      UseMask[i] = true;
    }
    return true;
  }
  void Verify(ValidationContext &ValCtx) {
    for (unsigned i = 0; i < Table.Entries; i++) {
      if (UseMask[i])
        continue;

      ValCtx.EmitFormatError(ValidationRule::ContainerUnusedItemInTable,
                             {"SemanticIndexTable", std::to_string(i)});
    }
  }
};

class PSVContentVerifier {
  DxilModule &DM;
  DxilPipelineStateValidation &PSV;
  ValidationContext &ValCtx;
  bool PSVContentValid = true;
  StringTableVerifier StrTableVerifier;
  SemanticIndexTableVerifier IndexTableVerifier;

public:
  PSVContentVerifier(DxilPipelineStateValidation &PSV, DxilModule &DM,
                     ValidationContext &ValCtx)
      : DM(DM), PSV(PSV), ValCtx(ValCtx),
        StrTableVerifier(PSV.GetStringTable()),
        IndexTableVerifier(PSV.GetSemanticIndexTable()) {}
  void Verify(unsigned ValMajor, unsigned ValMinor, unsigned PSVVersion);

private:
  void VerifySignatures(unsigned ValMajor, unsigned ValMinor);
  void VerifySignature(const DxilSignature &, PSVSignatureElement0 *Base,
                       unsigned Count, std::string Name,
                       bool i1ToUnknownCompat);
  void VerifySignatureElement(const DxilSignatureElement &,
                              PSVSignatureElement0 *, const PSVStringTable &,
                              const PSVSemanticIndexTable &, std::string, bool);
  void VerifyResources(unsigned PSVVersion);
  template <typename T>
  void VerifyResourceTable(T &ResTab, unsigned &ResourceIndex,
                           unsigned PSVVersion);
  void VerifyViewIDDependence(PSVRuntimeInfo1 *PSV1, unsigned PSVVersion);
  void VerifyEntryProperties(const ShaderModel *SM, PSVRuntimeInfo0 *PSV0,
                             PSVRuntimeInfo1 *PSV1, PSVRuntimeInfo2 *PSV2);
  void EmitMismatchError(StringRef Name, StringRef PartContent,
                         StringRef ModuleContent) {
    ValCtx.EmitFormatError(ValidationRule::ContainerContentMatches,
                           {Name, "PSV0", PartContent, ModuleContent});
    PSVContentValid = false;
  }
  void EmitInvalidError(StringRef Name) {
    ValCtx.EmitFormatError(ValidationRule::ContainerContentInvalid,
                           {"PSV0 part", Name});
    PSVContentValid = false;
  }
  template <typename Ty> static std::string GetDump(const Ty &T) {
    std::string Str;
    raw_string_ostream OS(Str);
    T.Print(OS);
    OS.flush();
    return Str;
  }
};

void PSVContentVerifier::VerifyViewIDDependence(PSVRuntimeInfo1 *PSV1,
                                                unsigned PSVVersion) {
  std::vector<unsigned int> ViewStateInPSV;
  unsigned OutputSizeInUInts = hlsl::LoadViewIDStateFromPSV(nullptr, 0, PSV);
  if (OutputSizeInUInts) {
    ViewStateInPSV.assign(OutputSizeInUInts, 0);
    hlsl::LoadViewIDStateFromPSV(ViewStateInPSV.data(),
                                 (unsigned)ViewStateInPSV.size(), PSV);
  }
  // In case the num of input/output scalars are aligned to 4, the
  // ViewStateInPSV could match the DxilModule's view state directly.
  std::vector<unsigned int> ViewStateInDxilModule =
      DM.GetSerializedViewIdState();
  if (ViewStateInPSV == ViewStateInDxilModule)
    return;
  if (ViewStateInDxilModule.empty() &&
      std::all_of(ViewStateInPSV.begin(), ViewStateInPSV.end(),
                  [](unsigned int i) { return i == 0; }))
    return;

  std::string Str;
  raw_string_ostream OS(Str);
  PSV.PrintViewIDState(OS);
  OS.flush();

  // Create a Temp PSV from DxilModule to print the ViewIDState.
  unique_ptr<DxilPartWriter> pWriter(NewPSVWriter(DM, PSVVersion));
  CComPtr<AbstractMemoryStream> pOutputStream;
  IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pOutputStream));
  pOutputStream->Reserve(pWriter->size());
  pWriter->write(pOutputStream);

  DxilPipelineStateValidation PSVFromDxilModule;
  if (!PSVFromDxilModule.InitFromPSV0(pOutputStream->GetPtr(),
                                      pOutputStream->GetPtrSize())) {
    ValCtx.EmitFormatError(
        ValidationRule::ContainerPartMatches,
        {"Pipeline State Validation generated from DxiModule"});
    return;
  }

  ViewStateInDxilModule.clear();
  OutputSizeInUInts =
      hlsl::LoadViewIDStateFromPSV(nullptr, 0, PSVFromDxilModule);
  if (OutputSizeInUInts) {
    ViewStateInDxilModule.assign(OutputSizeInUInts, 0);
    hlsl::LoadViewIDStateFromPSV(ViewStateInDxilModule.data(),
                                 OutputSizeInUInts, PSVFromDxilModule);
  }
  // ViewStateInDxilModule and ViewStateInPSV all go through
  // LoadViewIDStateFromPSV here, so they should match.
  if (ViewStateInPSV == ViewStateInDxilModule)
    return;

  std::string Str1;
  raw_string_ostream OS1(Str1);
  PSVFromDxilModule.PrintViewIDState(OS1);
  OS1.flush();
  EmitMismatchError("ViewIDState", Str, Str1);
}

void PSVContentVerifier::VerifySignatures(unsigned ValMajor,
                                          unsigned ValMinor) {
  bool i1ToUnknownCompat = DXIL::CompareVersions(ValMajor, ValMinor, 1, 5) < 0;
  // Verify input signature
  VerifySignature(DM.GetInputSignature(), PSV.GetInputElement0(0),
                  PSV.GetSigInputElements(), "SigInput", i1ToUnknownCompat);
  // Verify output signature
  VerifySignature(DM.GetOutputSignature(), PSV.GetOutputElement0(0),
                  PSV.GetSigOutputElements(), "SigOutput", i1ToUnknownCompat);
  // Verify patch constant signature
  VerifySignature(DM.GetPatchConstOrPrimSignature(),
                  PSV.GetPatchConstOrPrimElement0(0),
                  PSV.GetSigPatchConstOrPrimElements(),
                  "SigPatchConstantOrPrim", i1ToUnknownCompat);
}

void PSVContentVerifier::VerifySignature(const DxilSignature &Sig,
                                         PSVSignatureElement0 *Base,
                                         unsigned Count, std::string Name,
                                         bool i1ToUnknownCompat) {
  if (Count != Sig.GetElements().size()) {
    EmitMismatchError(Name + "Elements", std::to_string(Count),
                      std::to_string(Sig.GetElements().size()));
    return;
  }

  // Verify each element in DxilSignature.
  const PSVStringTable &StrTab = PSV.GetStringTable();
  const PSVSemanticIndexTable &IndexTab = PSV.GetSemanticIndexTable();
  for (unsigned i = 0; i < Count; i++)
    VerifySignatureElement(Sig.GetElement(i),
                           PSV.GetRecord<PSVSignatureElement0>(
                               Base, PSV.GetSignatureElementSize(), Count, i),
                           StrTab, IndexTab, Name, i1ToUnknownCompat);
}

void PSVContentVerifier::VerifySignatureElement(
    const DxilSignatureElement &SE, PSVSignatureElement0 *PSVSE0,
    const PSVStringTable &StrTab, const PSVSemanticIndexTable &IndexTab,
    std::string Name, bool i1ToUnknownCompat) {
  bool InvalidTableAccess = false;
  if (!StrTableVerifier.MarkUse(PSVSE0->SemanticName)) {
    EmitInvalidError("SemanticName");
    InvalidTableAccess = true;
  }
  if (!IndexTableVerifier.MarkUse(PSVSE0->SemanticIndexes, PSVSE0->Rows)) {
    EmitInvalidError("SemanticIndex");
    InvalidTableAccess = true;
  }
  if (InvalidTableAccess)
    return;
  // Find the signature element in the set.
  PSVSignatureElement0 ModulePSVSE0;
  InitPSVSignatureElement(ModulePSVSE0, SE, i1ToUnknownCompat);

  // Check the Name and SemanticIndex.
  bool Mismatch = false;
  const std::vector<uint32_t> &SemanticIndexVec = SE.GetSemanticIndexVec();
  llvm::ArrayRef<uint32_t> PSVSemanticIndexVec(
      IndexTab.Get(PSVSE0->SemanticIndexes), PSVSE0->Rows);
  if (SemanticIndexVec.size() == PSVSemanticIndexVec.size())
    Mismatch |= memcmp(PSVSemanticIndexVec.data(), SemanticIndexVec.data(),
                       SemanticIndexVec.size() * sizeof(uint32_t)) != 0;
  else
    Mismatch = true;

  ModulePSVSE0.SemanticIndexes = PSVSE0->SemanticIndexes;

  PSVSignatureElement PSVSE(StrTab, IndexTab, PSVSE0);
  if (SE.IsArbitrary())
    Mismatch |= strcmp(PSVSE.GetSemanticName(), SE.GetName()) != 0;
  else
    Mismatch |= PSVSE0->SemanticKind != static_cast<uint8_t>(SE.GetKind());

  ModulePSVSE0.SemanticName = PSVSE0->SemanticName;
  // Compare all fields.
  Mismatch |= memcmp(&ModulePSVSE0, PSVSE0, sizeof(PSVSignatureElement0)) != 0;
  if (Mismatch) {
    PSVSignatureElement ModulePSVSE(StrTab, IndexTab, &ModulePSVSE0);
    std::string ModuleStr;
    raw_string_ostream OS(ModuleStr);
    ModulePSVSE.Print(OS, SE.GetName(), SemanticIndexVec.data());
    OS.flush();
    std::string PartStr;
    raw_string_ostream OS1(PartStr);
    PSVSE.Print(OS1, PSVSE.GetSemanticName(), PSVSemanticIndexVec.data());
    OS1.flush();
    EmitMismatchError(Name + "Element", PartStr, ModuleStr);
  }
}

template <typename T>
void PSVContentVerifier::VerifyResourceTable(T &ResTab, unsigned &ResourceIndex,
                                             unsigned PSVVersion) {
  for (auto &&R : ResTab) {
    PSVResourceBindInfo1 BI;
    InitPSVResourceBinding(&BI, &BI, R.get());

    if (PSVVersion > 1) {
      PSVResourceBindInfo1 *BindInfo =
          PSV.GetPSVResourceBindInfo1(ResourceIndex);
      if (memcmp(&BI, BindInfo, sizeof(PSVResourceBindInfo1)) != 0) {
        std::string ModuleStr = GetDump(BI);
        std::string PartStr = GetDump(*BindInfo);
        EmitMismatchError("ResourceBindInfo", PartStr, ModuleStr);
      }
    } else {
      PSVResourceBindInfo0 *BindInfo =
          PSV.GetPSVResourceBindInfo0(ResourceIndex);
      if (memcmp(&BI, BindInfo, sizeof(PSVResourceBindInfo0)) != 0) {
        std::string ModuleStr = GetDump(BI);
        std::string PartStr = GetDump(*BindInfo);
        EmitMismatchError("ResourceBindInfo", PartStr, ModuleStr);
      }
    }
    ResourceIndex++;
  }
}

void PSVContentVerifier::VerifyResources(unsigned PSVVersion) {
  UINT uCBuffers = DM.GetCBuffers().size();
  UINT uSamplers = DM.GetSamplers().size();
  UINT uSRVs = DM.GetSRVs().size();
  UINT uUAVs = DM.GetUAVs().size();
  unsigned ResourceCount = uCBuffers + uSamplers + uSRVs + uUAVs;
  if (PSV.GetBindCount() != ResourceCount) {
    EmitMismatchError("ResourceCount", std::to_string(PSV.GetBindCount()),
                      std::to_string(ResourceCount));
    return;
  }
  // Verify each resource table.
  unsigned ResIndex = 0;
  // CBV
  VerifyResourceTable(DM.GetCBuffers(), ResIndex, PSVVersion);
  // Sampler
  VerifyResourceTable(DM.GetSamplers(), ResIndex, PSVVersion);
  // SRV
  VerifyResourceTable(DM.GetSRVs(), ResIndex, PSVVersion);
  // UAV
  VerifyResourceTable(DM.GetUAVs(), ResIndex, PSVVersion);
}

void PSVContentVerifier::VerifyEntryProperties(const ShaderModel *SM,
                                               PSVRuntimeInfo0 *PSV0,
                                               PSVRuntimeInfo1 *PSV1,
                                               PSVRuntimeInfo2 *PSV2) {
  PSVRuntimeInfo3 DMPSV;
  memset(&DMPSV, 0, sizeof(PSVRuntimeInfo3));

  hlsl::SetShaderProps((PSVRuntimeInfo0 *)&DMPSV, DM);
  hlsl::SetShaderProps((PSVRuntimeInfo1 *)&DMPSV, DM);
  hlsl::SetShaderProps((PSVRuntimeInfo2 *)&DMPSV, DM);
  if (PSV1) {
    // Init things not set in InitPSVRuntimeInfo.
    DMPSV.ShaderStage = static_cast<uint8_t>(SM->GetKind());
    DMPSV.SigInputElements = DM.GetInputSignature().GetElements().size();
    DMPSV.SigOutputElements = DM.GetOutputSignature().GetElements().size();
    DMPSV.SigPatchConstOrPrimElements =
        DM.GetPatchConstOrPrimSignature().GetElements().size();
    // Set up ViewID and signature dependency info
    DMPSV.UsesViewID = DM.m_ShaderFlags.GetViewID() ? true : false;
    DMPSV.SigInputVectors = DM.GetInputSignature().NumVectorsUsed(0);
    for (unsigned streamIndex = 0; streamIndex < 4; streamIndex++)
      DMPSV.SigOutputVectors[streamIndex] =
          DM.GetOutputSignature().NumVectorsUsed(streamIndex);
    if (SM->IsHS() || SM->IsDS() || SM->IsMS())
      DMPSV.SigPatchConstOrPrimVectors =
          DM.GetPatchConstOrPrimSignature().NumVectorsUsed(0);
  }
  bool Mismatched = false;
  if (PSV2)
    Mismatched = memcmp(PSV2, &DMPSV, sizeof(PSVRuntimeInfo2)) != 0;
  else if (PSV1)
    Mismatched = memcmp(PSV1, &DMPSV, sizeof(PSVRuntimeInfo1)) != 0;
  else
    Mismatched = memcmp(PSV0, &DMPSV, sizeof(PSVRuntimeInfo0)) != 0;

  if (Mismatched) {
    std::string Str;
    raw_string_ostream OS(Str);
    hlsl::PrintPSVRuntimeInfo(OS, &DMPSV, &DMPSV, &DMPSV, &DMPSV,
                              static_cast<uint8_t>(SM->GetKind()),
                              DM.GetEntryFunctionName().c_str(), "");
    OS.flush();
    std::string Str1;
    raw_string_ostream OS1(Str1);
    PSV.PrintPSVRuntimeInfo(OS1, static_cast<uint8_t>(PSVShaderKind::Library),
                            "");
    OS1.flush();
    EmitMismatchError("PSVRuntimeInfo", Str, Str1);
  }
}

void PSVContentVerifier::Verify(unsigned ValMajor, unsigned ValMinor,
                                unsigned PSVVersion) {
  PSVInitInfo PSVInfo(PSVVersion);

  if (PSV.GetBindCount() > 0 &&
      PSV.GetResourceBindInfoSize() != PSVInfo.ResourceBindInfoSize()) {
    EmitMismatchError("ResourceBindInfoSize",
                      std::to_string(PSV.GetResourceBindInfoSize()),
                      std::to_string(PSVInfo.ResourceBindInfoSize()));
    return;
  }
  VerifyResources(PSVVersion);

  PSVRuntimeInfo0 *PSV0 = PSV.GetPSVRuntimeInfo0();
  PSVRuntimeInfo1 *PSV1 = PSV.GetPSVRuntimeInfo1();
  PSVRuntimeInfo2 *PSV2 = PSV.GetPSVRuntimeInfo2();

  const ShaderModel *SM = DM.GetShaderModel();
  VerifyEntryProperties(SM, PSV0, PSV1, PSV2);
  if (PSVVersion > 0) {
    if (((PSV.GetSigInputElements() + PSV.GetSigOutputElements() +
          PSV.GetSigPatchConstOrPrimElements()) > 0) &&
        PSV.GetSignatureElementSize() != PSVInfo.SignatureElementSize()) {
      EmitMismatchError("SignatureElementSize",
                        std::to_string(PSV.GetSignatureElementSize()),
                        std::to_string(PSVInfo.SignatureElementSize()));
      return;
    }
    uint8_t ShaderStage = static_cast<uint8_t>(SM->GetKind());
    if (PSV1->ShaderStage != ShaderStage) {
      EmitMismatchError("ShaderStage", std::to_string(PSV1->ShaderStage),
                        std::to_string(ShaderStage));
      return;
    }
    bool ViewIDUsed = PSV1->UsesViewID != 0;
    if (ViewIDUsed != DM.m_ShaderFlags.GetViewID())
      EmitMismatchError("UsesViewID", std::to_string(PSV1->UsesViewID),
                        std::to_string(DM.m_ShaderFlags.GetViewID()));

    VerifySignatures(ValMajor, ValMinor);

    VerifyViewIDDependence(PSV1, PSVVersion);
  }
  // PSV2 only added NumThreadsX/Y/Z which verified in VerifyEntryProperties.
  if (PSVVersion > 2) {
    PSVRuntimeInfo3 *PSV3 = PSV.GetPSVRuntimeInfo3();
    if (!StrTableVerifier.MarkUse(PSV3->EntryFunctionName)) {
      EmitInvalidError("EntryFunctionName");
    } else {
      if (DM.GetEntryFunctionName() != PSV.GetEntryFunctionName())
        EmitMismatchError("EntryFunctionName", PSV.GetEntryFunctionName(),
                          DM.GetEntryFunctionName());
    }
  }

  StrTableVerifier.Verify(ValCtx);
  IndexTableVerifier.Verify(ValCtx);

  if (!PSVContentValid)
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches,
                           {"Pipeline State Validation"});
}

} // namespace

namespace hlsl {

// DXIL Container Verification Functions

static void VerifyBlobPartMatches(ValidationContext &ValCtx, LPCSTR pName,
                                  DxilPartWriter *pWriter, const void *pData,
                                  uint32_t Size) {
  if (!pData && pWriter->size()) {
    // No blob part, but writer says non-zero size is expected.
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMissing, {pName});
    return;
  }

  // Compare sizes
  if (pWriter->size() != Size) {
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches, {pName});
    return;
  }

  if (Size == 0) {
    return;
  }

  CComPtr<AbstractMemoryStream> pOutputStream;
  IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pOutputStream));
  pOutputStream->Reserve(Size);

  pWriter->write(pOutputStream);
  DXASSERT(pOutputStream->GetPtrSize() == Size,
           "otherwise, DxilPartWriter misreported size");

  if (memcmp(pData, pOutputStream->GetPtr(), Size)) {
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches, {pName});
    return;
  }

  return;
}

static void VerifySignatureMatches(ValidationContext &ValCtx,
                                   DXIL::SignatureKind SigKind,
                                   const void *pSigData, uint32_t SigSize) {
  // Generate corresponding signature from module and memcmp

  const char *pName = nullptr;
  switch (SigKind) {
  case hlsl::DXIL::SignatureKind::Input:
    pName = "Program Input Signature";
    break;
  case hlsl::DXIL::SignatureKind::Output:
    pName = "Program Output Signature";
    break;
  case hlsl::DXIL::SignatureKind::PatchConstOrPrim:
    if (ValCtx.DxilMod.GetShaderModel()->GetKind() == DXIL::ShaderKind::Mesh)
      pName = "Program Primitive Signature";
    else
      pName = "Program Patch Constant Signature";
    break;
  default:
    break;
  }

  unique_ptr<DxilPartWriter> pWriter(
      NewProgramSignatureWriter(ValCtx.DxilMod, SigKind));
  VerifyBlobPartMatches(ValCtx, pName, pWriter.get(), pSigData, SigSize);
}

bool VerifySignatureMatches(llvm::Module *pModule, DXIL::SignatureKind SigKind,
                            const void *pSigData, uint32_t SigSize) {
  ValidationContext ValCtx(*pModule, nullptr, pModule->GetOrCreateDxilModule());
  VerifySignatureMatches(ValCtx, SigKind, pSigData, SigSize);
  return !ValCtx.Failed;
}

struct SimplePSV {
  uint32_t PSVRuntimeInfoSize = 0;
  uint32_t PSVNumResources = 0;
  uint32_t PSVResourceBindInfoSize = 0;
  uint32_t StringTableSize = 0;
  const char *StringTable = nullptr;
  uint32_t SemanticIndexTableEntries = 0;
  const uint32_t *SemanticIndexTable = nullptr;
  uint32_t PSVSignatureElementSize = 0;
  const PSVRuntimeInfo1 *RuntimeInfo1 = nullptr;
  bool IsValid = true;
  SimplePSV(const void *pPSVData, uint32_t PSVSize) {

#define INCREMENT_POS(Size)                                                    \
  Offset += Size;                                                              \
  if (Offset > PSVSize) {                                                      \
    IsValid = false;                                                           \
    return;                                                                    \
  }

    uint32_t Offset = 0;
    PSVRuntimeInfoSize = GetUint32AtOffset(pPSVData, 0);
    INCREMENT_POS(4);
    if (PSVRuntimeInfoSize >= sizeof(PSVRuntimeInfo1))
      RuntimeInfo1 =
          (const PSVRuntimeInfo1 *)(GetPtrAtOffset(pPSVData, Offset));
    INCREMENT_POS(PSVRuntimeInfoSize);

    PSVNumResources = GetUint32AtOffset(pPSVData, Offset);
    INCREMENT_POS(4);
    if (PSVNumResources > 0) {
      PSVResourceBindInfoSize = GetUint32AtOffset(pPSVData, Offset);
      // Increase the offset for the resource bind info size.
      INCREMENT_POS(4);
      // Increase the offset for the resource bind info.
      INCREMENT_POS(PSVNumResources * PSVResourceBindInfoSize);
    }
    if (RuntimeInfo1) {
      StringTableSize = GetUint32AtOffset(pPSVData, Offset);
      INCREMENT_POS(4);
      // Make sure StringTableSize is aligned to 4 bytes.
      if ((StringTableSize & 3) != 0) {
        IsValid = false;
        return;
      }
      if (StringTableSize) {
        StringTable = GetPtrAtOffset(pPSVData, Offset);
        INCREMENT_POS(StringTableSize);
      }
      SemanticIndexTableEntries = GetUint32AtOffset(pPSVData, Offset);
      INCREMENT_POS(4);
      if (SemanticIndexTableEntries) {
        SemanticIndexTable =
            (const uint32_t *)(GetPtrAtOffset(pPSVData, Offset));
        INCREMENT_POS(SemanticIndexTableEntries * 4);
      }
      if (RuntimeInfo1->SigInputElements || RuntimeInfo1->SigOutputElements ||
          RuntimeInfo1->SigPatchConstOrPrimElements) {
        PSVSignatureElementSize = GetUint32AtOffset(pPSVData, Offset);
        INCREMENT_POS(4);
        uint32_t PSVNumSignatures = RuntimeInfo1->SigInputElements +
                                    RuntimeInfo1->SigOutputElements +
                                    RuntimeInfo1->SigPatchConstOrPrimElements;
        INCREMENT_POS(PSVNumSignatures * PSVSignatureElementSize);
      }
      if (RuntimeInfo1->UsesViewID) {
        for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
          uint32_t SigOutputVectors = RuntimeInfo1->SigOutputVectors[i];
          if (SigOutputVectors == 0)
            continue;
          uint32_t MaskSizeInBytes =
              sizeof(uint32_t) *
              PSVComputeMaskDwordsFromVectors(SigOutputVectors);
          INCREMENT_POS(MaskSizeInBytes);
        }
        if ((RuntimeInfo1->ShaderStage == (unsigned)DXIL::ShaderKind::Hull ||
             RuntimeInfo1->ShaderStage == (unsigned)DXIL::ShaderKind::Mesh) &&
            RuntimeInfo1->SigPatchConstOrPrimVectors) {
          uint32_t MaskSizeInBytes =
              sizeof(uint32_t) * PSVComputeMaskDwordsFromVectors(
                                     RuntimeInfo1->SigPatchConstOrPrimVectors);
          INCREMENT_POS(MaskSizeInBytes);
        }
      }

      for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
        uint32_t SigOutputVectors = RuntimeInfo1->SigOutputVectors[i];
        if (SigOutputVectors == 0)
          continue;
        uint32_t TableSizeInBytes =
            sizeof(uint32_t) *
            PSVComputeInputOutputTableDwords(RuntimeInfo1->SigInputVectors,
                                             SigOutputVectors);
        INCREMENT_POS(TableSizeInBytes);
      }

      if ((RuntimeInfo1->ShaderStage == (unsigned)DXIL::ShaderKind::Hull ||
           RuntimeInfo1->ShaderStage == (unsigned)DXIL::ShaderKind::Mesh) &&
          RuntimeInfo1->SigPatchConstOrPrimVectors &&
          RuntimeInfo1->SigInputVectors) {
        uint32_t TableSizeInBytes =
            sizeof(uint32_t) * PSVComputeInputOutputTableDwords(
                                   RuntimeInfo1->SigInputVectors,
                                   RuntimeInfo1->SigPatchConstOrPrimVectors);
        INCREMENT_POS(TableSizeInBytes);
      }

      if (RuntimeInfo1->ShaderStage == (unsigned)DXIL::ShaderKind::Domain &&
          RuntimeInfo1->SigOutputVectors[0] &&
          RuntimeInfo1->SigPatchConstOrPrimVectors) {
        uint32_t TableSizeInBytes =
            sizeof(uint32_t) * PSVComputeInputOutputTableDwords(
                                   RuntimeInfo1->SigPatchConstOrPrimVectors,
                                   RuntimeInfo1->SigOutputVectors[0]);
        INCREMENT_POS(TableSizeInBytes);
      }
    }
    IsValid = PSVSize == Offset;
#undef INCREMENT_POS
  }
  bool ValidatePSVInit(PSVInitInfo PSVInfo, ValidationContext &ValCtx) {
    if (PSVRuntimeInfoSize != PSVInfo.RuntimeInfoSize()) {
      ValCtx.EmitFormatError(ValidationRule::ContainerContentMatches,
                             {"PSVRuntimeInfoSize", "PSV0",
                              std::to_string(PSVRuntimeInfoSize),
                              std::to_string(PSVInfo.RuntimeInfoSize())});
      return false;
    }
    if (PSVNumResources &&
        PSVResourceBindInfoSize != PSVInfo.ResourceBindInfoSize()) {
      ValCtx.EmitFormatError(ValidationRule::ContainerContentMatches,
                             {"PSVResourceBindInfoSize", "PSV0",
                              std::to_string(PSVResourceBindInfoSize),
                              std::to_string(PSVInfo.ResourceBindInfoSize())});
      return false;
    }
    if (RuntimeInfo1 &&
        (RuntimeInfo1->SigInputElements || RuntimeInfo1->SigOutputElements ||
         RuntimeInfo1->SigPatchConstOrPrimElements) &&
        PSVSignatureElementSize != PSVInfo.SignatureElementSize()) {
      ValCtx.EmitFormatError(ValidationRule::ContainerContentMatches,
                             {"PSVSignatureElementSize", "PSV0",
                              std::to_string(PSVSignatureElementSize),
                              std::to_string(PSVInfo.SignatureElementSize())});
      return false;
    }
    return true;
  }

private:
  const char *GetPtrAtOffset(const void *BasePtr, uint32_t Offset) const {
    return (const char *)BasePtr + Offset;
  }
  uint32_t GetUint32AtOffset(const void *BasePtr, uint32_t Offset) const {
    return *(const uint32_t *)GetPtrAtOffset(BasePtr, Offset);
  }
};

static void VerifyPSVMatches(ValidationContext &ValCtx, const void *pPSVData,
                             uint32_t PSVSize) {
  // SimplePSV.IsValid indicates whether the part is well-formed so that we may
  // proceed with more detailed validation.
  SimplePSV SimplePSV(pPSVData, PSVSize);
  if (!SimplePSV.IsValid) {
    ValCtx.EmitFormatError(ValidationRule::ContainerContentInvalid,
                           {"DxilContainer", "PSV0 part"});
    return;
  }
  // The PSVVersion determines the size of record structures that should be
  // used when writing PSV0 data, and is based on the validator version in the
  // module.
  unsigned ValMajor, ValMinor;
  ValCtx.DxilMod.GetValidatorVersion(ValMajor, ValMinor);
  unsigned PSVVersion = hlsl::GetPSVVersion(ValMajor, ValMinor);
  // PSVInfo is used to compute the expected record size of the PSV0 part of the
  // container. It uses facts from the module.
  PSVInitInfo PSVInfo(PSVVersion);
  hlsl::SetupPSVInitInfo(PSVInfo, ValCtx.DxilMod);
  // ValidatePSVInit checks that record sizes match expected for PSVVersion.
  if (!SimplePSV.ValidatePSVInit(PSVInfo, ValCtx))
    return;
  // Ensure that the string table data is null-terminated.
  if (SimplePSV.StringTable &&
      SimplePSV.StringTable[SimplePSV.StringTableSize - 1] != '\0') {
    ValCtx.EmitFormatError(ValidationRule::ContainerContentInvalid,
                           {"PSV part StringTable"});
    return;
  }

  DxilPipelineStateValidation PSV;
  if (!PSV.InitFromPSV0(pPSVData, PSVSize)) {
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches,
                           {"Pipeline State Validation"});
    return;
  }

  PSVContentVerifier Verifier(PSV, ValCtx.DxilMod, ValCtx);
  Verifier.Verify(ValMajor, ValMinor, PSVVersion);
}

static void VerifyFeatureInfoMatches(ValidationContext &ValCtx,
                                     const void *pFeatureInfoData,
                                     uint32_t FeatureInfoSize) {
  // generate Feature Info data from module and memcmp
  unique_ptr<DxilPartWriter> pWriter(NewFeatureInfoWriter(ValCtx.DxilMod));
  VerifyBlobPartMatches(ValCtx, "Feature Info", pWriter.get(), pFeatureInfoData,
                        FeatureInfoSize);
}

// return true if the pBlob is a valid, well-formed CompilerVersion part, false
// otherwise
bool ValidateCompilerVersionPart(const void *pBlobPtr, UINT blobSize) {
  // The hlsl::DxilCompilerVersion struct is always 16 bytes. (2 2-byte
  // uint16's, 3 4-byte uint32's) The blob size should absolutely never be less
  // than 16 bytes.
  if (blobSize < sizeof(hlsl::DxilCompilerVersion)) {
    return false;
  }

  const hlsl::DxilCompilerVersion *pDCV =
      (const hlsl::DxilCompilerVersion *)pBlobPtr;
  if (pDCV->VersionStringListSizeInBytes == 0) {
    // No version strings, just make sure there is no extra space.
    return blobSize == sizeof(hlsl::DxilCompilerVersion);
  }

  // after this point, we know VersionStringListSizeInBytes >= 1, because it is
  // a UINT

  UINT EndOfVersionStringIndex =
      sizeof(hlsl::DxilCompilerVersion) + pDCV->VersionStringListSizeInBytes;
  // Make sure that the buffer size is large enough to contain both the DCV
  // struct and the version string but not any larger than necessary
  if (PSVALIGN4(EndOfVersionStringIndex) != blobSize) {
    return false;
  }

  const char *VersionStringsListData =
      (const char *)pBlobPtr + sizeof(hlsl::DxilCompilerVersion);
  UINT VersionStringListSizeInBytes = pDCV->VersionStringListSizeInBytes;

  // now make sure that any pad bytes that were added are null-terminators.
  for (UINT i = VersionStringListSizeInBytes;
       i < blobSize - sizeof(hlsl::DxilCompilerVersion); i++) {
    if (VersionStringsListData[i] != '\0') {
      return false;
    }
  }

  // Now, version string validation
  // first, the final byte of the string should always be null-terminator so
  // that the string ends
  if (VersionStringsListData[VersionStringListSizeInBytes - 1] != '\0') {
    return false;
  }

  // construct the first string
  // data format for VersionString can be see in the definition for the
  // DxilCompilerVersion struct. summary: 2 strings that each end with the null
  // terminator, and [0-3] null terminators after the final null terminator
  StringRef firstStr(VersionStringsListData);

  // if the second string exists, attempt to construct it.
  if (VersionStringListSizeInBytes > (firstStr.size() + 1)) {
    StringRef secondStr(VersionStringsListData + firstStr.size() + 1);

    // the VersionStringListSizeInBytes member should be exactly equal to the
    // two string lengths, plus the 2 null terminator bytes.
    if (VersionStringListSizeInBytes !=
        firstStr.size() + secondStr.size() + 2) {
      return false;
    }
  } else {
    // the VersionStringListSizeInBytes member should be exactly equal to the
    // first string length, plus the 1 null terminator byte.
    if (VersionStringListSizeInBytes != firstStr.size() + 1) {
      return false;
    }
  }

  return true;
}

static void VerifyRDATMatches(ValidationContext &ValCtx, const void *pRDATData,
                              uint32_t RDATSize) {
  const char *PartName = "Runtime Data (RDAT)";
  RDAT::DxilRuntimeData rdat(pRDATData, RDATSize);
  if (!rdat.Validate()) {
    ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches, {PartName});
    return;
  }

  // If DxilModule subobjects already loaded, validate these against the RDAT
  // blob, otherwise, load subobject into DxilModule to generate reference RDAT.
  if (!ValCtx.DxilMod.GetSubobjects()) {
    auto table = rdat.GetSubobjectTable();
    if (table && table.Count() > 0) {
      ValCtx.DxilMod.ResetSubobjects(new DxilSubobjects());
      if (!LoadSubobjectsFromRDAT(*ValCtx.DxilMod.GetSubobjects(), rdat)) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches,
                               {PartName});
        return;
      }
    }
  }

  unique_ptr<DxilPartWriter> pWriter(NewRDATWriter(ValCtx.DxilMod));
  VerifyBlobPartMatches(ValCtx, PartName, pWriter.get(), pRDATData, RDATSize);
}

bool VerifyRDATMatches(llvm::Module *pModule, const void *pRDATData,
                       uint32_t RDATSize) {
  ValidationContext ValCtx(*pModule, nullptr, pModule->GetOrCreateDxilModule());
  VerifyRDATMatches(ValCtx, pRDATData, RDATSize);
  return !ValCtx.Failed;
}

bool VerifyFeatureInfoMatches(llvm::Module *pModule,
                              const void *pFeatureInfoData,
                              uint32_t FeatureInfoSize) {
  ValidationContext ValCtx(*pModule, nullptr, pModule->GetOrCreateDxilModule());
  VerifyFeatureInfoMatches(ValCtx, pFeatureInfoData, FeatureInfoSize);
  return !ValCtx.Failed;
}

HRESULT ValidateDxilContainerParts(llvm::Module *pModule,
                                   llvm::Module *pDebugModule,
                                   const DxilContainerHeader *pContainer,
                                   uint32_t ContainerSize) {

  DXASSERT_NOMSG(pModule);
  if (!pContainer || !IsValidDxilContainer(pContainer, ContainerSize)) {
    return DXC_E_CONTAINER_INVALID;
  }

  DxilModule *pDxilModule = DxilModule::TryGetDxilModule(pModule);
  if (!pDxilModule) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  ValidationContext ValCtx(*pModule, pDebugModule, *pDxilModule);

  DXIL::ShaderKind ShaderKind = pDxilModule->GetShaderModel()->GetKind();
  bool bTessOrMesh = ShaderKind == DXIL::ShaderKind::Hull ||
                     ShaderKind == DXIL::ShaderKind::Domain ||
                     ShaderKind == DXIL::ShaderKind::Mesh;

  std::unordered_set<uint32_t> FourCCFound;
  const DxilPartHeader *pRootSignaturePart = nullptr;
  const DxilPartHeader *pPSVPart = nullptr;

  for (auto it = begin(pContainer), itEnd = end(pContainer); it != itEnd;
       ++it) {
    const DxilPartHeader *pPart = *it;

    char szFourCC[5];
    PartKindToCharArray(pPart->PartFourCC, szFourCC);
    if (FourCCFound.find(pPart->PartFourCC) != FourCCFound.end()) {
      // Two parts with same FourCC found
      ValCtx.EmitFormatError(ValidationRule::ContainerPartRepeated, {szFourCC});
      continue;
    }
    FourCCFound.insert(pPart->PartFourCC);

    switch (pPart->PartFourCC) {
    case DFCC_InputSignature:
      if (ValCtx.isLibProfile) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      } else {
        VerifySignatureMatches(ValCtx, DXIL::SignatureKind::Input,
                               GetDxilPartData(pPart), pPart->PartSize);
      }
      break;
    case DFCC_OutputSignature:
      if (ValCtx.isLibProfile) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      } else {
        VerifySignatureMatches(ValCtx, DXIL::SignatureKind::Output,
                               GetDxilPartData(pPart), pPart->PartSize);
      }
      break;
    case DFCC_PatchConstantSignature:
      if (ValCtx.isLibProfile) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      } else {
        if (bTessOrMesh) {
          VerifySignatureMatches(ValCtx, DXIL::SignatureKind::PatchConstOrPrim,
                                 GetDxilPartData(pPart), pPart->PartSize);
        } else {
          ValCtx.EmitFormatError(ValidationRule::ContainerPartMatches,
                                 {"Program Patch Constant Signature"});
        }
      }
      break;
    case DFCC_FeatureInfo:
      VerifyFeatureInfoMatches(ValCtx, GetDxilPartData(pPart), pPart->PartSize);
      break;
    case DFCC_CompilerVersion:
      // This blob is either a PDB, or a library profile
      if (ValCtx.isLibProfile) {
        if (!ValidateCompilerVersionPart((void *)GetDxilPartData(pPart),
                                         pPart->PartSize)) {
          ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                                 {szFourCC});
        }
      } else {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      }
      break;

    case DFCC_RootSignature:
      pRootSignaturePart = pPart;
      if (ValCtx.isLibProfile) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      }
      break;
    case DFCC_PipelineStateValidation:
      pPSVPart = pPart;
      if (ValCtx.isLibProfile) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      } else {
        VerifyPSVMatches(ValCtx, GetDxilPartData(pPart), pPart->PartSize);
      }
      break;

    // Skip these
    case DFCC_ResourceDef:
    case DFCC_ShaderStatistics:
    case DFCC_PrivateData:
      break;
    case DFCC_DXIL:
    case DFCC_ShaderDebugInfoDXIL: {
      const DxilProgramHeader *pProgramHeader =
          reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(pPart));
      if (!pProgramHeader)
        continue;

      int PV = pProgramHeader->ProgramVersion;
      int major = (PV >> 4) & 0xF; // Extract the major version (next 4 bits)
      int minor = PV & 0xF;        // Extract the minor version (lowest 4 bits)

      int moduleMajor = pDxilModule->GetShaderModel()->GetMajor();
      int moduleMinor = pDxilModule->GetShaderModel()->GetMinor();
      if (moduleMajor != major || moduleMinor != minor) {
        ValCtx.EmitFormatError(ValidationRule::SmProgramVersion,
                               {std::to_string(major), std::to_string(minor),
                                std::to_string(moduleMajor),
                                std::to_string(moduleMinor)});
        return DXC_E_INCORRECT_PROGRAM_VERSION;
      }
      continue;
    }
    case DFCC_ShaderDebugName:
      continue;

    case DFCC_ShaderHash:
      if (pPart->PartSize != sizeof(DxilShaderHash)) {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      }
      break;

    // Runtime Data (RDAT) for libraries
    case DFCC_RuntimeData:
      if (ValCtx.isLibProfile) {
        // TODO: validate without exact binary comparison of serialized data
        //  - support earlier versions
        //  - verify no newer record versions than known here (size no larger
        //  than newest version)
        //  - verify all data makes sense and matches expectations based on
        //  module
        VerifyRDATMatches(ValCtx, GetDxilPartData(pPart), pPart->PartSize);
      } else {
        ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid,
                               {szFourCC});
      }
      break;

    case DFCC_Container:
    default:
      ValCtx.EmitFormatError(ValidationRule::ContainerPartInvalid, {szFourCC});
      break;
    }
  }

  // Verify required parts found
  if (ValCtx.isLibProfile) {
    if (FourCCFound.find(DFCC_RuntimeData) == FourCCFound.end()) {
      ValCtx.EmitFormatError(ValidationRule::ContainerPartMissing,
                             {"Runtime Data (RDAT)"});
    }
  } else {
    if (FourCCFound.find(DFCC_InputSignature) == FourCCFound.end()) {
      VerifySignatureMatches(ValCtx, DXIL::SignatureKind::Input, nullptr, 0);
    }
    if (FourCCFound.find(DFCC_OutputSignature) == FourCCFound.end()) {
      VerifySignatureMatches(ValCtx, DXIL::SignatureKind::Output, nullptr, 0);
    }
    if (bTessOrMesh &&
        FourCCFound.find(DFCC_PatchConstantSignature) == FourCCFound.end() &&
        pDxilModule->GetPatchConstOrPrimSignature().GetElements().size()) {
      ValCtx.EmitFormatError(ValidationRule::ContainerPartMissing,
                             {"Program Patch Constant Signature"});
    }
    if (FourCCFound.find(DFCC_FeatureInfo) == FourCCFound.end()) {
      // Could be optional, but RS1 runtime doesn't handle this case properly.
      ValCtx.EmitFormatError(ValidationRule::ContainerPartMissing,
                             {"Feature Info"});
    }

    // Validate Root Signature
    if (pPSVPart) {
      if (pRootSignaturePart) {
        std::string diagStr;
        raw_string_ostream DiagStream(diagStr);
        try {
          RootSignatureHandle RS;
          RS.LoadSerialized(
              (const uint8_t *)GetDxilPartData(pRootSignaturePart),
              pRootSignaturePart->PartSize);
          RS.Deserialize();
          IFTBOOL(VerifyRootSignatureWithShaderPSV(
                      RS.GetDesc(), pDxilModule->GetShaderModel()->GetKind(),
                      GetDxilPartData(pPSVPart), pPSVPart->PartSize,
                      DiagStream),
                  DXC_E_INCORRECT_ROOT_SIGNATURE);
        } catch (...) {
          ValCtx.EmitError(ValidationRule::ContainerRootSignatureIncompatible);
          emitDxilDiag(pModule->getContext(), DiagStream.str().c_str());
        }
      }
    } else {
      ValCtx.EmitFormatError(ValidationRule::ContainerPartMissing,
                             {"Pipeline State Validation"});
    }
  }

  if (ValCtx.Failed) {
    return DXC_E_MALFORMED_CONTAINER;
  }
  return S_OK;
}

static HRESULT FindDxilPart(const void *pContainerBytes, uint32_t ContainerSize,
                            DxilFourCC FourCC, const DxilPartHeader **ppPart) {

  const DxilContainerHeader *pContainer =
      IsDxilContainerLike(pContainerBytes, ContainerSize);

  if (!pContainer) {
    IFR(DXC_E_CONTAINER_INVALID);
  }
  if (!IsValidDxilContainer(pContainer, ContainerSize)) {
    IFR(DXC_E_CONTAINER_INVALID);
  }

  DxilPartIterator it =
      std::find_if(begin(pContainer), end(pContainer), DxilPartIsType(FourCC));
  if (it == end(pContainer)) {
    IFR(DXC_E_CONTAINER_MISSING_DXIL);
  }

  const DxilProgramHeader *pProgramHeader =
      reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(*it));
  if (!IsValidDxilProgramHeader(pProgramHeader, (*it)->PartSize)) {
    IFR(DXC_E_CONTAINER_INVALID);
  }

  *ppPart = *it;
  return S_OK;
}

HRESULT ValidateLoadModule(const char *pIL, uint32_t ILLength,
                           unique_ptr<llvm::Module> &pModule, LLVMContext &Ctx,
                           llvm::raw_ostream &DiagStream, unsigned bLazyLoad) {

  llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
  PrintDiagnosticContext DiagContext(DiagPrinter);
  DiagRestore DR(Ctx, &DiagContext);

  std::unique_ptr<llvm::MemoryBuffer> pBitcodeBuf;
  pBitcodeBuf.reset(llvm::MemoryBuffer::getMemBuffer(
                        llvm::StringRef(pIL, ILLength), "", false)
                        .release());

  ErrorOr<std::unique_ptr<Module>> loadedModuleResult =
      bLazyLoad == 0
          ? llvm::parseBitcodeFile(pBitcodeBuf->getMemBufferRef(), Ctx, nullptr,
                                   true /*Track Bitstream*/)
          : llvm::getLazyBitcodeModule(std::move(pBitcodeBuf), Ctx, nullptr,
                                       false, true /*Track Bitstream*/);

  // DXIL disallows some LLVM bitcode constructs, like unaccounted-for
  // sub-blocks. These appear as warnings, which the validator should reject.
  if (DiagContext.HasErrors() || DiagContext.HasWarnings() ||
      loadedModuleResult.getError())
    return DXC_E_IR_VERIFICATION_FAILED;

  pModule = std::move(loadedModuleResult.get());
  return S_OK;
}

HRESULT ValidateDxilBitcode(const char *pIL, uint32_t ILLength,
                            llvm::raw_ostream &DiagStream) {

  LLVMContext Ctx;
  std::unique_ptr<llvm::Module> pModule;

  llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
  PrintDiagnosticContext DiagContext(DiagPrinter);
  Ctx.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                           &DiagContext, true);

  HRESULT hr;
  if (FAILED(hr = ValidateLoadModule(pIL, ILLength, pModule, Ctx, DiagStream,
                                     /*bLazyLoad*/ false)))
    return hr;

  if (FAILED(hr = ValidateDxilModule(pModule.get(), nullptr)))
    return hr;

  DxilModule &dxilModule = pModule->GetDxilModule();
  auto &SerializedRootSig = dxilModule.GetSerializedRootSignature();
  if (!SerializedRootSig.empty()) {
    unique_ptr<DxilPartWriter> pWriter(NewPSVWriter(dxilModule));
    DXASSERT_NOMSG(pWriter->size());
    CComPtr<AbstractMemoryStream> pOutputStream;
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pOutputStream));
    pOutputStream->Reserve(pWriter->size());
    pWriter->write(pOutputStream);
    DxilVersionedRootSignature desc;
    try {
      DeserializeRootSignature(SerializedRootSig.data(),
                               SerializedRootSig.size(), desc.get_address_of());
      if (!desc.get()) {
        return DXC_E_INCORRECT_ROOT_SIGNATURE;
      }
      IFTBOOL(VerifyRootSignatureWithShaderPSV(
                  desc.get(), dxilModule.GetShaderModel()->GetKind(),
                  pOutputStream->GetPtr(), pWriter->size(), DiagStream),
              DXC_E_INCORRECT_ROOT_SIGNATURE);
    } catch (...) {
      return DXC_E_INCORRECT_ROOT_SIGNATURE;
    }
  }

  if (DiagContext.HasErrors() || DiagContext.HasWarnings()) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  return S_OK;
}

static HRESULT ValidateLoadModuleFromContainer(
    const void *pContainer, uint32_t ContainerSize,
    std::unique_ptr<llvm::Module> &pModule,
    std::unique_ptr<llvm::Module> &pDebugModule, llvm::LLVMContext &Ctx,
    LLVMContext &DbgCtx, llvm::raw_ostream &DiagStream, unsigned bLazyLoad) {
  llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
  PrintDiagnosticContext DiagContext(DiagPrinter);
  DiagRestore DR(Ctx, &DiagContext);
  DiagRestore DR2(DbgCtx, &DiagContext);

  const DxilPartHeader *pPart = nullptr;
  IFR(FindDxilPart(pContainer, ContainerSize, DFCC_DXIL, &pPart));

  const char *pIL = nullptr;
  uint32_t ILLength = 0;
  GetDxilProgramBitcode(
      reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(pPart)), &pIL,
      &ILLength);

  IFR(ValidateLoadModule(pIL, ILLength, pModule, Ctx, DiagStream, bLazyLoad));

  HRESULT hr;
  const DxilPartHeader *pDbgPart = nullptr;
  if (FAILED(hr = FindDxilPart(pContainer, ContainerSize,
                               DFCC_ShaderDebugInfoDXIL, &pDbgPart)) &&
      hr != DXC_E_CONTAINER_MISSING_DXIL) {
    return hr;
  }

  if (pDbgPart) {
    GetDxilProgramBitcode(
        reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(pDbgPart)),
        &pIL, &ILLength);
    if (FAILED(hr = ValidateLoadModule(pIL, ILLength, pDebugModule, DbgCtx,
                                       DiagStream, bLazyLoad))) {
      return hr;
    }
  }

  return S_OK;
}

HRESULT ValidateLoadModuleFromContainer(
    const void *pContainer, uint32_t ContainerSize,
    std::unique_ptr<llvm::Module> &pModule,
    std::unique_ptr<llvm::Module> &pDebugModule, llvm::LLVMContext &Ctx,
    llvm::LLVMContext &DbgCtx, llvm::raw_ostream &DiagStream) {
  return ValidateLoadModuleFromContainer(pContainer, ContainerSize, pModule,
                                         pDebugModule, Ctx, DbgCtx, DiagStream,
                                         /*bLazyLoad*/ false);
}
// Lazy loads module from container, validating load, but not module.
HRESULT ValidateLoadModuleFromContainerLazy(
    const void *pContainer, uint32_t ContainerSize,
    std::unique_ptr<llvm::Module> &pModule,
    std::unique_ptr<llvm::Module> &pDebugModule, llvm::LLVMContext &Ctx,
    llvm::LLVMContext &DbgCtx, llvm::raw_ostream &DiagStream) {
  return ValidateLoadModuleFromContainer(pContainer, ContainerSize, pModule,
                                         pDebugModule, Ctx, DbgCtx, DiagStream,
                                         /*bLazyLoad*/ true);
}

HRESULT ValidateDxilContainer(const void *pContainer, uint32_t ContainerSize,
                              llvm::Module *pDebugModule,
                              llvm::raw_ostream &DiagStream) {
  LLVMContext Ctx, DbgCtx;
  std::unique_ptr<llvm::Module> pModule, pDebugModuleInContainer;

  llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
  PrintDiagnosticContext DiagContext(DiagPrinter);
  Ctx.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                           &DiagContext, true);
  DbgCtx.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                              &DiagContext, true);

  DiagRestore DR(pDebugModule, &DiagContext);

  IFR(ValidateLoadModuleFromContainer(pContainer, ContainerSize, pModule,
                                      pDebugModuleInContainer, Ctx, DbgCtx,
                                      DiagStream));

  if (pDebugModuleInContainer)
    pDebugModule = pDebugModuleInContainer.get();

  // Validate DXIL Module
  IFR(ValidateDxilModule(pModule.get(), pDebugModule));

  if (DiagContext.HasErrors() || DiagContext.HasWarnings()) {
    return DXC_E_IR_VERIFICATION_FAILED;
  }

  return ValidateDxilContainerParts(
      pModule.get(), pDebugModule,
      IsDxilContainerLike(pContainer, ContainerSize), ContainerSize);
}

HRESULT ValidateDxilContainer(const void *pContainer, uint32_t ContainerSize,
                              llvm::raw_ostream &DiagStream) {
  return ValidateDxilContainer(pContainer, ContainerSize, nullptr, DiagStream);
}
} // namespace hlsl
