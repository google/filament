///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcdxrfallbackcompiler.cpp                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Raytracing Fallback Compiler object.               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
// Includes on Windows are highly order dependent.
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcdxrfallbackcompiler.h"
#include "dxc/DxrFallback/DxrFallbackCompiler.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/HLSL/DxilLinker.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilValidation/DxilValidation.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxcutil.h"


#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LegacyPassManager.h"

#include "dxc/HLSL/DxilFallbackLayerPass.h"
// clang-format on

using namespace llvm;
using namespace hlsl;

static std::string ws2s(const std::wstring &wide) {
  return std::string(wide.begin(), wide.end());
}

static HRESULT FindDxilProgram(IDxcBlob *pBlob, DxilFourCC FourCC,
                               const DxilProgramHeader **ppProgram) {

  void *pContainerBytes = pBlob->GetBufferPointer();
  SIZE_T ContainerSize = pBlob->GetBufferSize();
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

  *ppProgram = pProgramHeader;
  return S_OK;
}

static DxilModule *ExtractDxil(LLVMContext &context, IDxcBlob *pContainer) {
  const DxilProgramHeader *pProgram = nullptr;
  IFT(FindDxilProgram(pContainer, DFCC_DXIL, &pProgram));

  const char *pIL = nullptr;
  uint32_t ILLength = 0;
  GetDxilProgramBitcode(pProgram, &pIL, &ILLength);

  std::unique_ptr<Module> M;
  std::string diagStr;
  M = dxilutil::LoadModuleFromBitcode(llvm::StringRef(pIL, ILLength), context,
                                      diagStr);

  DxilModule *dxil = nullptr;
  if (M)
    dxil = &M->GetOrCreateDxilModule();
  M.release();

  return dxil;
}

static void saveModuleToAsmFile(const llvm::Module *mod,
                                const std::string &filename) {
  std::error_code EC;
  raw_fd_ostream out(filename, EC, sys::fs::F_Text);
  if (!out.has_error()) {
    mod->print(out, nullptr);
    out.close();
  }
  if (out.has_error()) {
    errs() << "Error saving to " << filename << ":" << filename << "\n";
    exit(1);
  }
}

class DxcDxrFallbackCompiler : public IDxcDxrFallbackCompiler {
private:
  DXC_MICROCOM_TM_REF_FIELDS()
  bool m_findCalledShaders = false;
  int m_debugOutput = 0;

  // Only used for test purposes when exports aren't explicitly listed
  std::unique_ptr<DxrFallbackCompiler::IntToFuncNameMap> m_pCachedMap;

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcDxrFallbackCompiler)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcDxrFallbackCompiler>(this, iid, ppvObject);
  }

  HRESULT STDMETHODCALLTYPE SetFindCalledShaders(bool val) override {
    m_findCalledShaders = val;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE SetDebugOutput(int val) override {
    m_debugOutput = val;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE PatchShaderBindingTables(
      const LPCWSTR pEntryName, DxcShaderBytecode *pShaderBytecode,
      void *pShaderInfo, IDxcOperationResult **ppResult) override;

  HRESULT STDMETHODCALLTYPE RenameAndLink(
      DxcShaderBytecode *pLibs, UINT32 libCount, DxcExportDesc *pExports,
      UINT32 ExportCount, IDxcOperationResult **ppResult) override;

  HRESULT STDMETHODCALLTYPE Compile(DxcShaderBytecode *pLibs, UINT32 libCount,
                                    const LPCWSTR *pShaderNames,
                                    DxcShaderInfo *pShaderInfo,
                                    UINT32 shaderCount, UINT32 maxAttributeSize,
                                    IDxcOperationResult **ppResult) override;

  HRESULT STDMETHODCALLTYPE Link(const LPCWSTR pEntryName, IDxcBlob **pLibs,
                                 UINT32 libCount, const LPCWSTR *pShaderNames,
                                 DxcShaderInfo *pShaderInfo, UINT32 shaderCount,
                                 UINT32 maxAttributeSize,
                                 UINT32 stackSizeInBytes,
                                 IDxcOperationResult **ppResult) override;
};

// TODO: Stolen from Brandon's code, merge
// Remove ELF mangling
static inline std::string GetUnmangledName(StringRef name) {
  if (!name.startswith("\x1?"))
    return name;

  size_t pos = name.find("@@");
  if (pos == name.npos)
    return name;

  return name.substr(2, pos - 2);
}

static Function *getFunctionFromName(Module &M,
                                     const std::wstring &exportName) {
  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    std::wstring functionName = Unicode::UTF8ToWideStringOrThrow(
        GetUnmangledName(F->getName()).c_str());
    if (exportName == functionName) {
      return F;
    }
  }
  return nullptr;
}

DXIL::ShaderKind getRayShaderKind(Function *F);
Function *CloneFunction(Function *Orig, const llvm::Twine &Name,
                        llvm::Module *llvmModule);

HRESULT STDMETHODCALLTYPE DxcDxrFallbackCompiler::RenameAndLink(
    DxcShaderBytecode *pLibs, UINT32 libCount, DxcExportDesc *pExports,
    UINT32 ExportCount, IDxcOperationResult **ppResult) {
  if (pLibs == nullptr || pExports == nullptr)
    return E_POINTER;

  if (libCount == 0 || ExportCount == 0)
    return E_INVALIDARG;

  *ppResult = nullptr;
  HRESULT hr = S_OK;
  DxcThreadMalloc TM(m_pMalloc);
  LLVMContext context;
  try {
    // Init file system because we are currently loading the runtime from disk
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    // Create a diagnostic printer
    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pDiagStream));
    raw_stream_ostream DiagStream(pDiagStream);
    DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    context.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                                 &DiagContext, true);

    std::vector<CComPtr<IDxcBlobEncoding>> pShaderLibs(libCount);
    for (UINT i = 0; i < libCount; i++) {
      hlsl::DxcCreateBlobWithEncodingFromPinned(pLibs[i].pData, pLibs[i].Size,
                                                CP_ACP, &pShaderLibs[i]);
    }

    // Link all the modules together into a single into library
    unsigned int valMajor = 0, valMinor = 0;
    dxcutil::GetValidatorVersion(&valMajor, &valMinor);

    std::unique_ptr<Module> M;
    {
      DxilLinker *pLinker =
          DxilLinker::CreateLinker(context, valMajor, valMinor);
      for (UINT32 i = 0; i < libCount; ++i) {
        DxilModule *dxil = ExtractDxil(context, pShaderLibs[i]);
        if (dxil == nullptr) {
          return DXC_E_CONTAINER_MISSING_DXIL;
        }
        pLinker->RegisterLib(std::to_string(i),
                             std::unique_ptr<Module>(dxil->GetModule()),
                             nullptr);
        pLinker->AttachLib(std::to_string(i));
      }

      dxilutil::ExportMap exportMap;
      M = pLinker->Link("", "lib_6_3", exportMap);
      if (m_debugOutput) {
        saveModuleToAsmFile(M.get(), "combined.ll");
      }
    }

    dxilutil::ExportMap exportMap;
    for (UINT i = 0; i < ExportCount; i++) {
      auto &exportDesc = pExports[i];
      auto exportName = ws2s(exportDesc.ExportName);
      if (exportDesc.ExportToRename) {
        auto exportToRename = ws2s(exportDesc.ExportToRename);
        CloneFunction(M->getFunction(exportToRename), exportName, M.get());
      }
      exportMap.Add(GetUnmangledName(exportName));
    }

    // Create the compute shader
    DxilLinker *pLinker = DxilLinker::CreateLinker(context, valMajor, valMinor);
    pLinker->RegisterLib("M", std::move(M), nullptr);
    pLinker->AttachLib("M");
    auto profile = "lib_6_3";
    M = pLinker->Link(StringRef(), profile, exportMap);
    bool hasErrors = DiagContext.HasErrors();

    CComPtr<IDxcBlob> pResultBlob;
    if (M) {
      CComPtr<AbstractMemoryStream> pOutputStream;
      IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pOutputStream));
      raw_stream_ostream outStream(pOutputStream.p);
      WriteBitcodeToFile(M.get(), outStream);
      outStream.flush();

      // Validation.
      dxcutil::AssembleInputs inputs(std::move(M), pResultBlob,
                                     TM.GetInstalledAllocator(),
                                     SerializeDxilFlags::None, pOutputStream);
      dxcutil::AssembleToContainer(inputs);
    }

    DiagStream.flush();
    CComPtr<IStream> pStream = static_cast<CComPtr<IStream>>(pDiagStream);
    std::string warnings;
    dxcutil::CreateOperationResultFromOutputs(pResultBlob, pStream, warnings,
                                              hasErrors, ppResult);
  }
  CATCH_CPP_ASSIGN_HRESULT();

  return hr;
}

HRESULT STDMETHODCALLTYPE DxcDxrFallbackCompiler::PatchShaderBindingTables(
    const LPCWSTR pEntryName, DxcShaderBytecode *pShaderBytecode,
    void *pShaderInfo, IDxcOperationResult **ppResult) {
  if (pShaderBytecode == nullptr || pShaderInfo == nullptr)
    return E_POINTER;

  *ppResult = nullptr;
  HRESULT hr = S_OK;
  DxcThreadMalloc TM(m_pMalloc);
  LLVMContext context;
  try {
    CComPtr<IDxcBlobEncoding> pShaderBlob;
    hlsl::DxcCreateBlobWithEncodingFromPinned(
        pShaderBytecode->pData, pShaderBytecode->Size, CP_ACP, &pShaderBlob);

    // Init file system because we are currently loading the runtime from disk
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    // Create a diagnostic printer
    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pDiagStream));
    raw_stream_ostream DiagStream(pDiagStream);
    DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    context.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                                 &DiagContext, true);

    DxilModule *dxil = ExtractDxil(context, pShaderBlob);

    // TODO: Lifetime managment?
    std::unique_ptr<Module> M(dxil->GetModule());
    if (dxil == nullptr) {
      return DXC_E_CONTAINER_MISSING_DXIL;
    }

    ModulePass *patchShaderRecordBindingsPass =
        createDxilPatchShaderRecordBindingsPass();

    char dxilPatchShaderRecordString[32];
    StringCchPrintf(dxilPatchShaderRecordString,
                    _countof(dxilPatchShaderRecordString), "%p", pShaderInfo);
    auto passOption = PassOption("root-signature", dxilPatchShaderRecordString);
    PassOptions options(passOption);
    patchShaderRecordBindingsPass->applyOptions(options);

    legacy::PassManager FPM;
    FPM.add(patchShaderRecordBindingsPass);
    FPM.run(*M);

    CComPtr<IDxcBlob> pResultBlob;
    if (M) {
      CComPtr<AbstractMemoryStream> pOutputStream;
      IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pOutputStream));
      raw_stream_ostream outStream(pOutputStream.p);
      WriteBitcodeToFile(M.get(), outStream);
      outStream.flush();
      dxcutil::AssembleInputs inputs(std::move(M), pResultBlob,
                                     TM.GetInstalledAllocator(),
                                     SerializeDxilFlags::None, pOutputStream);
      dxcutil::AssembleToContainer(inputs);
    }

    DiagStream.flush();
    CComPtr<IStream> pStream = static_cast<CComPtr<IStream>>(pDiagStream);
    std::string warnings;
    dxcutil::CreateOperationResultFromOutputs(pResultBlob, pStream, warnings,
                                              false, ppResult);
  }
  CATCH_CPP_ASSIGN_HRESULT();

  return hr;
}

HRESULT STDMETHODCALLTYPE DxcDxrFallbackCompiler::Link(
    const LPCWSTR pEntryName, IDxcBlob **pLibs, UINT32 libCount,
    const LPCWSTR *pShaderNames, DxcShaderInfo *pShaderInfo, UINT32 shaderCount,
    UINT32 maxAttributeSize, UINT32 stackSizeInBytes,
    IDxcOperationResult **ppResult) {
  if (pLibs == nullptr || pShaderNames == nullptr || ppResult == nullptr)
    return E_POINTER;

  if (libCount == 0 || shaderCount == 0)
    return E_INVALIDARG;

  *ppResult = nullptr;
  HRESULT hr = S_OK;
  DxcThreadMalloc TM(m_pMalloc);
  LLVMContext context;
  try {
    // Init file system because we are currently loading the runtime from disk
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    // Create a diagnostic printer
    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pDiagStream));
    raw_stream_ostream DiagStream(pDiagStream);
    DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    context.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                                 &DiagContext, true);

    std::vector<std::string> shaderNames(shaderCount);
    for (UINT32 i = 0; i < shaderCount; ++i)
      shaderNames[i] = ws2s(pShaderNames[i]);

    // Link all the modules together into a single into library
    unsigned int valMajor = 0, valMinor = 0;
    dxcutil::GetValidatorVersion(&valMajor, &valMinor);
    std::unique_ptr<Module> M;
    {
      DxilLinker *pLinker =
          DxilLinker::CreateLinker(context, valMajor, valMinor);
      for (UINT32 i = 0; i < libCount; ++i) {
        DxilModule *dxil = ExtractDxil(context, pLibs[i]);
        if (dxil == nullptr) {
          return DXC_E_CONTAINER_MISSING_DXIL;
        }
        pLinker->RegisterLib(std::to_string(i),
                             std::unique_ptr<Module>(dxil->GetModule()),
                             nullptr);
        pLinker->AttachLib(std::to_string(i));
      }

      dxilutil::ExportMap exportMap;
      M = pLinker->Link("", "lib_6_3", exportMap);
      if (m_debugOutput) {
        saveModuleToAsmFile(M.get(), "combined.ll");
      }
    }

    std::vector<int> shaderEntryStateIds;
    std::vector<unsigned int> shaderStackSizes;

    DxrFallbackCompiler compiler(M.get(), shaderNames, maxAttributeSize,
                                 stackSizeInBytes, m_findCalledShaders);
    compiler.setDebugOutputLevel(m_debugOutput);
    shaderEntryStateIds.resize(shaderCount);
    shaderStackSizes.resize(shaderCount);
    for (UINT i = 0; i < shaderCount; i++) {
      shaderEntryStateIds[i] = pShaderInfo[i].Identifier;
      shaderStackSizes[i] = pShaderInfo[i].StackSize;
    }
    compiler.link(shaderEntryStateIds, shaderStackSizes, m_pCachedMap.get());
    if (m_debugOutput) {
      saveModuleToAsmFile(M.get(), "compiled.ll");
    }

    // Create the compute shader
    dxilutil::ExportMap exportMap;
    DxilLinker *pLinker = DxilLinker::CreateLinker(context, valMajor, valMinor);
    pLinker->RegisterLib("M", std::move(M), nullptr);
    pLinker->AttachLib("M");
    auto profile = "cs_6_0";
    M = pLinker->Link(pEntryName ? ws2s(pEntryName).c_str() : StringRef(),
                      profile, exportMap);
    bool hasErrors = DiagContext.HasErrors();

    CComPtr<IDxcBlob> pResultBlob;
    if (M) {
      if (!hasErrors && stackSizeInBytes)
        DxrFallbackCompiler::resizeStack(
            M->getFunction(ws2s(pEntryName).c_str()), stackSizeInBytes);

      llvm::NamedMDNode *IdentMetadata =
          M->getOrInsertNamedMetadata("llvm.ident");
      llvm::LLVMContext &Ctx = M->getContext();
      llvm::Metadata *IdentNode[] = {llvm::MDString::get(Ctx, "FallbackLayer")};
      IdentMetadata->addOperand(llvm::MDNode::get(Ctx, IdentNode));

      DxilModule &DM = M->GetDxilModule();
      DM.SetValidatorVersion(valMajor, valMinor);
      DxilModule::ClearDxilMetadata(*M);
      DM.EmitDxilMetadata();

      if (m_debugOutput)
        saveModuleToAsmFile(M.get(), "linked.ll");

#if !DISABLE_GET_CUSTOM_DIAG_ID
      const IntrusiveRefCntPtr<clang::DiagnosticIDs> Diags(
          new clang::DiagnosticIDs);
      IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts =
          new clang::DiagnosticOptions();
      // Construct our diagnostic client.
      clang::TextDiagnosticPrinter *DiagClient =
          new clang::TextDiagnosticPrinter(DiagStream, &*DiagOpts);
      clang::DiagnosticsEngine Diag(Diags, &*DiagOpts, DiagClient);
#endif
    }

    if (M) {
      CComPtr<AbstractMemoryStream> pOutputStream;
      IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pOutputStream));
      raw_stream_ostream outStream(pOutputStream.p);
      WriteBitcodeToFile(M.get(), outStream);
      outStream.flush();

      // Validation.
      dxcutil::AssembleInputs inputs(std::move(M), pResultBlob,
                                     TM.GetInstalledAllocator(),
                                     SerializeDxilFlags::None, pOutputStream,
                                     /*bDebugInfo*/ false);
      HRESULT valHR = dxcutil::ValidateAndAssembleToContainer(inputs);

      if (FAILED(valHR))
        hasErrors = true;
    }

    DiagStream.flush();
    CComPtr<IStream> pStream = static_cast<CComPtr<IStream>>(pDiagStream);
    std::string warnings;
    dxcutil::CreateOperationResultFromOutputs(pResultBlob, pStream, warnings,
                                              hasErrors, ppResult);
  }
  CATCH_CPP_ASSIGN_HRESULT();

  return hr;
}

HRESULT STDMETHODCALLTYPE DxcDxrFallbackCompiler::Compile(
    DxcShaderBytecode *pShaderLibs, UINT32 libCount,
    const LPCWSTR *pShaderNames, DxcShaderInfo *pShaderInfo, UINT32 shaderCount,
    UINT32 maxAttributeSize, IDxcOperationResult **ppResult) {
  if (pShaderLibs == nullptr || pShaderNames == nullptr || ppResult == nullptr)
    return E_POINTER;

  if (libCount == 0 || shaderCount == 0)
    return E_INVALIDARG;

  *ppResult = nullptr;
  HRESULT hr = S_OK;
  DxcThreadMalloc TM(m_pMalloc);
  LLVMContext context;
  try {
    std::vector<CComPtr<IDxcBlobEncoding>> pLibs(libCount);
    for (UINT i = 0; i < libCount; i++) {
      auto &shaderBytecode = pShaderLibs[i];
      hlsl::DxcCreateBlobWithEncodingFromPinned(
          shaderBytecode.pData, shaderBytecode.Size, CP_ACP, &pLibs[i]);
    }

    // Init file system because we are currently loading the runtime from disk
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    IFT(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    // Create a diagnostic printer
    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pDiagStream));
    raw_stream_ostream DiagStream(pDiagStream);
    DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    context.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                                 &DiagContext, true);

    std::vector<std::string> shaderNames(shaderCount);
    for (UINT32 i = 0; i < shaderCount; ++i)
      shaderNames[i] = ws2s(pShaderNames[i]);

    // Link all the modules together into a single into library
    unsigned int valMajor = 0, valMinor = 0;
    dxcutil::GetValidatorVersion(&valMajor, &valMinor);
    std::unique_ptr<Module> M;
    {
      DxilLinker *pLinker =
          DxilLinker::CreateLinker(context, valMajor, valMinor);
      for (UINT32 i = 0; i < libCount; ++i) {
        DxilModule *dxil = ExtractDxil(context, pLibs[i]);
        if (dxil == nullptr) {
          return DXC_E_CONTAINER_MISSING_DXIL;
        }
        pLinker->RegisterLib(std::to_string(i),
                             std::unique_ptr<Module>(dxil->GetModule()),
                             nullptr);
        pLinker->AttachLib(std::to_string(i));
      }

      dxilutil::ExportMap exportMap;
      M = pLinker->Link("", "lib_6_3", exportMap);
      if (m_debugOutput) {
        saveModuleToAsmFile(M.get(), "combined.ll");
      }
    }
    std::vector<ShaderType> shaderTypes;
    for (UINT32 i = 0; i < shaderCount; ++i) {
      switch (getRayShaderKind(getFunctionFromName(*M, pShaderNames[i]))) {
      case DXIL::ShaderKind::RayGeneration:
        shaderTypes.push_back(ShaderType::Raygen);
        break;
      case DXIL::ShaderKind::AnyHit:
        shaderTypes.push_back(ShaderType::AnyHit);
        break;
      case DXIL::ShaderKind::ClosestHit:
        shaderTypes.push_back(ShaderType::ClosestHit);
        break;
      case DXIL::ShaderKind::Intersection:
        shaderTypes.push_back(ShaderType::Intersection);
        break;
      case DXIL::ShaderKind::Miss:
        shaderTypes.push_back(ShaderType::Miss);
        break;
      case DXIL::ShaderKind::Callable:
        shaderTypes.push_back(ShaderType::Callable);
        break;
      default:
        shaderTypes.push_back(ShaderType::Lib);
        break;
      }
    }

    if (m_findCalledShaders) {
      m_pCachedMap.reset(new DxrFallbackCompiler::IntToFuncNameMap);
    }

    std::vector<int> shaderEntryStateIds;
    std::vector<unsigned int> shaderStackSizes;
    DxrFallbackCompiler compiler(M.get(), shaderNames, maxAttributeSize, 0,
                                 m_findCalledShaders);
    compiler.setDebugOutputLevel(m_debugOutput);
    compiler.compile(shaderEntryStateIds, shaderStackSizes, m_pCachedMap.get());
    if (m_debugOutput) {
      saveModuleToAsmFile(M.get(), "compiled.ll");
    }

    // Create the compute shader
    dxilutil::ExportMap exportMap;
    DxilLinker *pLinker = DxilLinker::CreateLinker(context, valMajor, valMinor);
    pLinker->RegisterLib("M", std::move(M), nullptr);
    pLinker->AttachLib("M");
    auto profile = "lib_6_3";
    M = pLinker->Link(StringRef(), profile, exportMap);
    bool hasErrors = DiagContext.HasErrors();

    CComPtr<IDxcBlob> pResultBlob;
    if (M) {
      CComPtr<AbstractMemoryStream> pOutputStream;
      IFT(CreateMemoryStream(TM.GetInstalledAllocator(), &pOutputStream));
      raw_stream_ostream outStream(pOutputStream.p);
      WriteBitcodeToFile(M.get(), outStream);
      outStream.flush();
      dxcutil::AssembleInputs inputs(std::move(M), pResultBlob,
                                     TM.GetInstalledAllocator(),
                                     SerializeDxilFlags::None, pOutputStream);
      dxcutil::AssembleToContainer(inputs);
    }

    DiagStream.flush();
    CComPtr<IStream> pStream = static_cast<CComPtr<IStream>>(pDiagStream);
    std::string warnings;
    dxcutil::CreateOperationResultFromOutputs(pResultBlob, pStream, warnings,
                                              hasErrors, ppResult);

    // Write out shader identifiers
    size_t copyCount = (m_findCalledShaders) ? 1 : shaderCount;
    for (unsigned int i = 0; i < copyCount; i++) {
      pShaderInfo[i].Identifier = shaderEntryStateIds[i];
      pShaderInfo[i].StackSize = shaderStackSizes[i];
      pShaderInfo[i].Type = shaderTypes[i];
    }
  }
  CATCH_CPP_ASSIGN_HRESULT();

  return hr;
}

HRESULT CreateDxcDxrFallbackCompiler(REFIID riid, LPVOID *ppv) {
  CComPtr<DxcDxrFallbackCompiler> result =
      DxcDxrFallbackCompiler::Alloc(DxcGetThreadMallocNoRef());
  if (result == nullptr) {
    *ppv = nullptr;
    return E_OUTOFMEMORY;
  }

  return result.p->QueryInterface(riid, ppv);
}
