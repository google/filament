///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxclinker.cpp                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the Dxil Linker.                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/ErrorCodes.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"

#include "llvm/ADT/SmallVector.h"
#include <algorithm>

#include "dxc/DxilValidation/DxilValidation.h"
#include "dxc/HLSL/DxilLinker.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.internal.h"
#include "dxcutil.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace hlsl;
using namespace llvm;

// This declaration is used for the locally-linked validator.
HRESULT CreateDxcValidator(REFIID riid, LPVOID *ppv);

struct DeserializedDxilCompilerVersion {
  const hlsl::DxilCompilerVersion DCV;
  std::string commitHashStr;
  std::string versionStr;

  DeserializedDxilCompilerVersion(const DxilCompilerVersion *pDCV)
      : DCV(*pDCV) {
    // Assumes pDCV has been checked for safe parsing
    if (pDCV->VersionStringListSizeInBytes) {
      const char *pStr = (const char *)(pDCV + 1);
      commitHashStr.assign(pStr);
      if (commitHashStr.size() + 1 < pDCV->VersionStringListSizeInBytes)
        versionStr.assign(pStr + commitHashStr.size() + 1);
    }
  }

  DeserializedDxilCompilerVersion(DeserializedDxilCompilerVersion &&other)
      : DCV(other.DCV), commitHashStr(std::move(other.commitHashStr)),
        versionStr(std::move(other.versionStr)) {}

  bool operator<(const DeserializedDxilCompilerVersion &rhs) const {
    return std::tie(DCV.Major, DCV.Minor, DCV.CommitCount,
                    DCV.VersionStringListSizeInBytes, commitHashStr,
                    versionStr) < std::tie(rhs.DCV.Major, rhs.DCV.Minor,
                                           rhs.DCV.CommitCount,
                                           rhs.DCV.VersionStringListSizeInBytes,
                                           rhs.commitHashStr, rhs.versionStr);
  }

  std::string display() const {
    std::string ret;
    ret += "Version(" + std::to_string(DCV.Major) + "." +
           std::to_string(DCV.Minor) + ") ";
    ret += "commits(" + std::to_string(DCV.CommitCount) + ") ";
    ret += "sha(" + commitHashStr + ") ";
    ret += "version string: \"" + versionStr + "\"";
    ret += "\n";

    return ret;
  }
};

class DxcLinker : public IDxcLinker, public IDxcContainerEvent {
public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcLinker)

  // Register a library with name to ref it later.
  HRESULT RegisterLibrary(LPCWSTR pLibName, // Name of the library.
                          IDxcBlob *pLib    // Library to add.
                          ) override;

  // Links the shader and produces a shader blob that the Direct3D runtime can
  // use.
  HRESULT STDMETHODCALLTYPE Link(
      LPCWSTR pEntryName,            // Entry point name
      LPCWSTR pTargetProfile,        // shader profile to link
      const LPCWSTR *pLibNames,      // Array of library names to link
      UINT32 libCount,               // Number of libraries to link
      const LPCWSTR *pArguments,     // Array of pointers to arguments
      UINT32 argCount,               // Number of arguments
      IDxcOperationResult **ppResult // Linker output status, buffer, and errors
      ) override;

  HRESULT STDMETHODCALLTYPE RegisterDxilContainerEventHandler(
      IDxcContainerEventsHandler *pHandler, UINT64 *pCookie) override {
    DxcThreadMalloc TM(m_pMalloc);
    DXASSERT(m_pDxcContainerEventsHandler == nullptr,
             "else events handler is already registered");
    *pCookie = 1; // Only one EventsHandler supported
    m_pDxcContainerEventsHandler = pHandler;
    return S_OK;
  };
  HRESULT STDMETHODCALLTYPE
  UnRegisterDxilContainerEventHandler(UINT64 cookie) override {
    DxcThreadMalloc TM(m_pMalloc);
    DXASSERT(m_pDxcContainerEventsHandler != nullptr,
             "else unregister should not have been called");
    m_pDxcContainerEventsHandler.Release();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcLinker>(this, riid, ppvObject);
  }

  void Initialize() {
    UINT32 valMajor, valMinor;
    dxcutil::GetValidatorVersion(&valMajor, &valMinor);
    m_pLinker.reset(DxilLinker::CreateLinker(m_Ctx, valMajor, valMinor));
  }

  bool AddCompilerVersionMapEntry(LPCWSTR libName,
                                  const hlsl::DxilCompilerVersion *pDCV,
                                  uint32_t partSize) {
    // Make sure it's safe to parse pDCV
    bool valid = pDCV && partSize >= sizeof(hlsl::DxilCompilerVersion) &&
                 partSize - sizeof(hlsl::DxilCompilerVersion) >=
                     pDCV->VersionStringListSizeInBytes;
    if (valid && pDCV->VersionStringListSizeInBytes) {
      const char *vStr = (const char *)(pDCV + 1);
      valid = vStr[pDCV->VersionStringListSizeInBytes - 1] == 0;
    }

    DXASSERT(valid, "DxilCompilerVersion part malformed");
    if (!valid)
      return false;

    CW2A pUtf8LibName(libName);
    std::string libNameStr = std::string(pUtf8LibName);
    auto result =
        m_uniqueCompilerVersions.insert(DeserializedDxilCompilerVersion(pDCV));
    m_libNameToCompilerVersionPart[libNameStr] = &(*result.first);

    return true;
  }

  ~DxcLinker() {
    // Make sure DxilLinker is released before LLVMContext.
    m_pLinker.reset();
  }

private:
  DXC_MICROCOM_TM_REF_FIELDS()
  LLVMContext m_Ctx;
  std::unique_ptr<DxilLinker> m_pLinker;
  CComPtr<IDxcContainerEventsHandler> m_pDxcContainerEventsHandler;
  std::vector<CComPtr<IDxcBlob>> m_blobs; // Keep blobs live for lazy load.
  std::map<std::string, const DeserializedDxilCompilerVersion *>
      m_libNameToCompilerVersionPart;
  std::set<DeserializedDxilCompilerVersion> m_uniqueCompilerVersions;
};

HRESULT
DxcLinker::RegisterLibrary(LPCWSTR pLibName, // Name of the library.
                           IDxcBlob *pBlob   // Library to add.
) {
  if (!pLibName || !pBlob)
    return E_INVALIDARG;
  DXASSERT(m_pLinker.get(), "else Initialize() not called or failed silently");
  DxcThreadMalloc TM(m_pMalloc);
  // Prepare UTF8-encoded versions of API values.
  CW2A pUtf8LibName(pLibName);
  // Already exist lib with same name.
  if (m_pLinker->HasLibNameRegistered(pUtf8LibName.m_psz))
    return E_INVALIDARG;

  try {
    std::unique_ptr<llvm::Module> pModule, pDebugModule;

    CComPtr<AbstractMemoryStream> pDiagStream;
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pDiagStream));

    raw_stream_ostream DiagStream(pDiagStream);

    IFR(ValidateLoadModuleFromContainerLazy(
        pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pModule,
        pDebugModule, m_Ctx, m_Ctx, DiagStream));

    // add an entry into the library to compiler version part map
    const hlsl::DxilContainerHeader *pHeader = hlsl::IsDxilContainerLike(
        pBlob->GetBufferPointer(), pBlob->GetBufferSize());
    const DxilPartHeader *pDPH = hlsl::GetDxilPartByType(
        pHeader, hlsl::DxilFourCC::DFCC_CompilerVersion);
    if (pDPH) {
      const hlsl::DxilCompilerVersion *pDCV =
          (const hlsl::DxilCompilerVersion *)(pDPH + 1);
      // If the compiler version string is non-empty, add the struct to the
      // map
      if (!AddCompilerVersionMapEntry(pLibName, pDCV, pDPH->PartSize)) {
        return E_INVALIDARG;
      }
    }

    if (m_pLinker->RegisterLib(pUtf8LibName.m_psz, std::move(pModule),
                               std::move(pDebugModule))) {
      m_blobs.emplace_back(pBlob);
      return S_OK;
    } else {
      return E_INVALIDARG;
    }
  } catch (hlsl::Exception &) {
    return E_INVALIDARG;
  }
}

// Links the shader and produces a shader blob that the Direct3D runtime can
// use.
HRESULT STDMETHODCALLTYPE DxcLinker::Link(
    LPCWSTR pEntryName,            // Entry point name
    LPCWSTR pTargetProfile,        // shader profile to link
    const LPCWSTR *pLibNames,      // Array of library names to link
    UINT32 libCount,               // Number of libraries to link
    const LPCWSTR *pArguments,     // Array of pointers to arguments
    UINT32 argCount,               // Number of arguments
    IDxcOperationResult **ppResult // Linker output status, buffer, and errors
) {
  if (!pTargetProfile || !pLibNames || libCount == 0 || !ppResult)
    return E_INVALIDARG;
  DxcThreadMalloc TM(m_pMalloc);
  // Prepare UTF8-encoded versions of API values.
  CW2A pUtf8TargetProfile(pTargetProfile);
  CW2A pUtf8EntryPoint(pEntryName);

  CComPtr<AbstractMemoryStream> pOutputStream;

  // Detach previous libraries.
  m_pLinker->DetachAll();

  HRESULT hr = S_OK;
  try {
    CComPtr<IDxcBlob> pOutputBlob;
    CComPtr<AbstractMemoryStream> pDiagStream;

    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pOutputStream));

    // Read and validate options.
    int argCountInt;
    IFT(UIntToInt(argCount, &argCountInt));
    hlsl::options::MainArgs mainArgs(argCountInt,
                                     const_cast<LPCWSTR *>(pArguments), 0);
    hlsl::options::DxcOpts opts;
    CW2A pUtf8TargetProfile(pTargetProfile);
    // Set target profile before reading options and validate
    opts.TargetProfile = pUtf8TargetProfile.m_psz;
    bool finished;
    dxcutil::ReadOptsAndValidate(mainArgs, opts, pOutputStream, ppResult,
                                 finished);
    if (pEntryName)
      opts.EntryPoint = pUtf8EntryPoint.m_psz;
    if (finished) {
      return S_OK;
    }

    // IDxcResult output
    CComPtr<DxcResult> pResult = DxcResult::Alloc(m_pMalloc);
    IFT(pResult->SetEncoding(opts.DefaultTextCodePage));
    IFT(pResult->SetOutputName(DXC_OUT_REFLECTION, opts.OutputReflectionFile));
    IFT(pResult->SetOutputName(DXC_OUT_SHADER_HASH, opts.OutputShaderHashFile));
    IFT(pResult->SetOutputName(DXC_OUT_ERRORS, opts.OutputWarningsFile));
    IFT(pResult->SetOutputName(DXC_OUT_ROOT_SIGNATURE, opts.OutputRootSigFile));
    IFT(pResult->SetOutputName(DXC_OUT_OBJECT, opts.OutputObject));

    std::string warnings;
    // llvm::raw_string_ostream w(warnings);
    IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pDiagStream));
    raw_stream_ostream DiagStream(pDiagStream);
    llvm::DiagnosticPrinterRawOStream DiagPrinter(DiagStream);
    PrintDiagnosticContext DiagContext(DiagPrinter);
    m_Ctx.setDiagnosticHandler(PrintDiagnosticContext::PrintDiagnosticHandler,
                               &DiagContext, true);

    unsigned valMajor = 0, valMinor = 0;
    if (opts.ValVerMajor != UINT_MAX) {
      // user-specified validator version override
      valMajor = opts.ValVerMajor;
      valMinor = opts.ValVerMinor;
    } else {
      // Version from dxil.dll, or internal validator if unavailable
      dxcutil::GetValidatorVersion(&valMajor, &valMinor);
    }
    m_pLinker->SetValidatorVersion(valMajor, valMinor);

    // Root signature-only container validation is only supported on 1.5 and
    // above.
    bool validateRootSigContainer =
        DXIL::CompareVersions(valMajor, valMinor, 1, 5) >= 0;

    bool needsValidation = !opts.DisableValidation;
    // Disable validation if ValVerMajor is 0 (offline target, never validate),
    // or pre-release library targets lib_6_1/lib_6_2.
    if (opts.ValVerMajor == 0 || opts.TargetProfile == "lib_6_1" ||
        opts.TargetProfile == "lib_6_2") {
      needsValidation = false;
    }

    // Attach libraries.
    bool bSuccess = true;
    const DeserializedDxilCompilerVersion *cur_version = nullptr;
    const DeserializedDxilCompilerVersion *first_version = nullptr;

    std::string cur_lib_name;
    std::string first_lib_name;

    for (UINT32 i = 0; i < libCount; i++) {
      CW2A pUtf8LibName(pLibNames[i]);
      bSuccess &= m_pLinker->AttachLib(pUtf8LibName.m_psz);

      cur_lib_name = std::string(pUtf8LibName);

      // only libraries with compiler version parts are in the map
      auto result = m_libNameToCompilerVersionPart.find(cur_lib_name);

      if (result != m_libNameToCompilerVersionPart.end()) {
        cur_version = result->second;
      }

      if (i == 0) {
        first_lib_name = cur_lib_name;
        first_version = cur_version;
      }

      if (cur_version != first_version) {

        std::string errorMsg = "error: Cannot link libraries with "
                               "conflicting compiler versions.\n";

        std::string firstErrorStr =
            "note: library \"" + first_lib_name + "\" version: ";
        firstErrorStr += first_version ? first_version->display()
                                       : "No version info available";

        std::string secondErrorStr =
            "note: library \"" + cur_lib_name + "\" version: ";
        secondErrorStr +=
            cur_version ? cur_version->display() : "No version info available";

        errorMsg += firstErrorStr + secondErrorStr;
        DiagStream << errorMsg;
        bSuccess = false;
      }
    }

    dxilutil::ExportMap exportMap;
    bSuccess &= exportMap.ParseExports(opts.Exports, DiagStream);

    if (opts.ExportShadersOnly)
      exportMap.setExportShadersOnly(true);

    bool hasErrorOccurred = !bSuccess;
    if (bSuccess) {
      std::unique_ptr<Module> pM =
          m_pLinker->Link(opts.EntryPoint, pUtf8TargetProfile.m_psz, exportMap);
      if (pM) {
        const IntrusiveRefCntPtr<clang::DiagnosticIDs> Diags(
            new clang::DiagnosticIDs);
        IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts =
            new clang::DiagnosticOptions();
        // Construct our diagnostic client.
        clang::TextDiagnosticPrinter *DiagClient =
            new clang::TextDiagnosticPrinter(DiagStream, &*DiagOpts);
        clang::DiagnosticsEngine Diag(Diags, &*DiagOpts, DiagClient);

        raw_stream_ostream outStream(pOutputStream.p);
        // Create bitcode of M.
        WriteBitcodeToFile(pM.get(), outStream);
        outStream.flush();

        DxilShaderHash ShaderHashContent;
        CComPtr<AbstractMemoryStream> pReflectionStream;
        IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pReflectionStream));
        CComPtr<AbstractMemoryStream> pRootSigStream;
        IFT(CreateMemoryStream(DxcGetThreadMallocNoRef(), &pRootSigStream));

        SerializeDxilFlags SerializeFlags =
            hlsl::options::ComputeSerializeDxilFlags(opts);
        // Always save debug info. If lib has debug info, the link result will
        // have debug info.
        SerializeFlags |= SerializeDxilFlags::IncludeDebugNamePart;
        // Unless we want to strip it right away, include it in the container.
        if (!opts.StripDebug) {
          SerializeFlags |= SerializeDxilFlags::IncludeDebugInfoPart;
        }

        // Validation.
        HRESULT valHR = S_OK;
        dxcutil::AssembleInputs inputs(
            std::move(pM), pOutputBlob, DxcGetThreadMallocNoRef(),
            SerializeFlags, pOutputStream, 0, opts.DebugFile, &Diag,
            &ShaderHashContent, pReflectionStream, pRootSigStream, nullptr,
            nullptr);
        if (needsValidation) {
          valHR = dxcutil::ValidateAndAssembleToContainer(inputs);
        } else {
          dxcutil::AssembleToContainer(inputs);
        }
        // Callback after valid DXIL is produced
        if (SUCCEEDED(valHR)) {
          CComPtr<IDxcBlob> pTargetBlob;
          if (m_pDxcContainerEventsHandler != nullptr) {
            HRESULT hr = m_pDxcContainerEventsHandler->OnDxilContainerBuilt(
                pOutputBlob, &pTargetBlob);
            if (SUCCEEDED(hr) && pTargetBlob != nullptr) {
              std::swap(pOutputBlob, pTargetBlob);
            }
          }
          // TODO: DFCC_ShaderDebugName

          // DXC_OUT_REFLECTION
          if (pReflectionStream && pReflectionStream->GetPtrSize()) {
            CComPtr<IDxcBlob> pReflection;
            IFT(pReflectionStream->QueryInterface(&pReflection));
            IFT(pResult->SetOutputObject(DXC_OUT_REFLECTION, pReflection));
          }

          // DXC_OUT_ROOT_SIGNATURE
          if (pRootSigStream && pRootSigStream->GetPtrSize()) {
            CComPtr<IDxcBlob> pRootSignature;
            IFT(pRootSigStream->QueryInterface(&pRootSignature));
            if (validateRootSigContainer && needsValidation) {
              CComPtr<IDxcBlobEncoding> pValErrors;
              // Validation failure communicated through diagnostic error
              dxcutil::ValidateRootSignatureInContainer(pRootSignature, &Diag);
            }
            IFT(pResult->SetOutputObject(DXC_OUT_ROOT_SIGNATURE,
                                         pRootSignature));
          }

          // DXC_OUT_SHADER_HASH
          CComPtr<IDxcBlob> pHashBlob;
          IFT(hlsl::DxcCreateBlobOnHeapCopy(&ShaderHashContent,
                                            (UINT32)sizeof(ShaderHashContent),
                                            &pHashBlob));
          IFT(pResult->SetOutputObject(DXC_OUT_SHADER_HASH, pHashBlob));

          // DXC_OUT_OBJECT
          IFT(pResult->SetOutputObject(DXC_OUT_OBJECT, pOutputBlob));
        }

        hasErrorOccurred = Diag.hasErrorOccurred();

      } else {
        hasErrorOccurred = true;
      }
    }
    DiagStream.flush();
    CComPtr<IStream> pStream = static_cast<CComPtr<IStream>>(pDiagStream);
    CComPtr<IDxcBlob> pErrorBlob;
    IFT(pStream.QueryInterface(&pErrorBlob));
    if (IsBlobNullOrEmpty(pErrorBlob)) {
      // Add std err to warnings.
      IFT(pResult->SetOutputString(DXC_OUT_ERRORS, warnings.c_str(),
                                   warnings.size()));
    } else {
      IFT(pResult->SetOutputObject(DXC_OUT_ERRORS, pErrorBlob));
    }
    IFT(pResult->SetStatusAndPrimaryResult(hasErrorOccurred ? E_FAIL : S_OK,
                                           DXC_OUT_OBJECT));
    IFT(pResult->QueryInterface(IID_PPV_ARGS(ppResult)));
  }
  CATCH_CPP_ASSIGN_HRESULT();
  return hr;
}

HRESULT CreateDxcLinker(REFIID riid, LPVOID *ppv) {
  *ppv = nullptr;
  try {
    CComPtr<DxcLinker> result(DxcLinker::Alloc(DxcGetThreadMallocNoRef()));
    IFROOM(result.p);
    result->Initialize();
    return result.p->QueryInterface(riid, ppv);
  }
  CATCH_CPP_RETURN_HRESULT();
}
