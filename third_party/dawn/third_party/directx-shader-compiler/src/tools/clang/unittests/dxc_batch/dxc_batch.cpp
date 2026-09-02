///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxc_bach.cpp                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxc_back console program.                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// dxc_batch is a fork of dxc showing how to build multiple shaders while
// sharing library-fied intermediates

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include <string>
#include <vector>

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <chrono>
#include <comdef.h>
#include <ios>
#include <thread>
#include <unordered_map>

#include "llvm/Support//MSFileSystem.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

inline bool wcseq(LPCWSTR a, LPCWSTR b) {
  return (a == nullptr && b == nullptr) ||
         (a != nullptr && b != nullptr && wcscmp(a, b) == 0);
}
inline bool wcsieq(LPCWSTR a, LPCWSTR b) { return _wcsicmp(a, b) == 0; }

using namespace dxc;
using namespace llvm;
using namespace llvm::opt;
using namespace hlsl::options;

extern HRESULT WINAPI DxilD3DCompile2(
    LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
    LPCSTR pEntrypoint, LPCSTR pTarget,
    const DxcDefine *pDefines, // Array of defines
    UINT32 defineCount,        // Number of defines
    LPCWSTR *pArguments,       // Array of pointers to arguments
    UINT32 argCount,           // Number of arguments
    IDxcIncludeHandler *pInclude, IDxcOperationResult **ppOperationResult);

class DxcContext {

private:
  DxcOpts &m_Opts;
  SpecificDllLoader &m_dxcSupport;

  int ActOnBlob(IDxcBlob *pBlob);
  int ActOnBlob(IDxcBlob *pBlob, IDxcBlob *pDebugBlob, LPCWSTR pDebugBlobName);
  void UpdatePart(IDxcBlob *pBlob, IDxcBlob **ppResult);
  bool UpdatePartRequired();
  void WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                   llvm::Twine &pVariableName, LPCWSTR pPath);
  HRESULT ReadFileIntoPartContent(hlsl::DxilFourCC fourCC, LPCWSTR fileName,
                                  IDxcBlob **ppResult);

  void ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult);
  int VerifyRootSignature();

public:
  DxcContext(DxcOpts &Opts, SpecificDllLoader &dxcSupport)
      : m_Opts(Opts), m_dxcSupport(dxcSupport) {}

  int Compile(llvm::StringRef path, bool bLibLink);
  int DumpBinary();
  void Preprocess();
};

static void PrintHlslException(const ::hlsl::Exception &hlslException,
                               llvm::StringRef stage,
                               llvm::raw_string_ostream &errorStream) {
  errorStream << stage << " failed\n";
  try {
    errorStream << hlslException.what();
    if (hlslException.hr == DXC_E_DUPLICATE_PART) {
      errorStream << "dxc_batch failed : DXIL container already contains the "
                     "given part.";
    } else if (hlslException.hr == DXC_E_MISSING_PART) {
      errorStream << "dxc_batch failed : DXIL container does not contain the "
                     "given part.";
    } else if (hlslException.hr == DXC_E_CONTAINER_INVALID) {
      errorStream << "dxc_batch failed : Invalid DXIL container.";
    } else if (hlslException.hr == DXC_E_CONTAINER_MISSING_DXIL) {
      errorStream << "dxc_batch failed : DXIL container is missing DXIL part.";
    } else if (hlslException.hr == DXC_E_CONTAINER_MISSING_DEBUG) {
      errorStream
          << "dxc_batch failed : DXIL container is missing Debug Info part.";
    } else if (hlslException.hr == E_OUTOFMEMORY) {
      errorStream << "dxc_batch failed : Out of Memory.";
    } else if (hlslException.hr == E_INVALIDARG) {
      errorStream << "dxc_batch failed : Invalid argument.";
    } else {
      errorStream << "dxc_batch failed : error code 0x" << std::hex
                  << hlslException.hr << ".";
    }
    errorStream << "\n";
  } catch (...) {
    errorStream << "  unable to retrieve error message.\n";
  }
}

static int Compile(llvm::StringRef command, SpecificDllLoader &dxcSupport,
                   llvm::StringRef path, bool bLinkLib,
                   std::string &errorString) {
  // llvm::raw_string_ostream &errorStream) {
  const OptTable *optionTable = getHlslOptTable();
  llvm::SmallVector<llvm::StringRef, 4> args;
  command.split(args, " ", /*MaxSplit*/ -1, /*KeepEmpty*/ false);
  if (!path.empty()) {
    args.emplace_back("-I");
    args.emplace_back(path);
  }

  MainArgs argStrings(args);
  DxcOpts dxcOpts;
  llvm::raw_string_ostream errorStream(errorString);

  int retVal =
      ReadDxcOpts(optionTable, DxcFlags, argStrings, dxcOpts, errorStream);

  if (0 == retVal) {
    try {
      DxcContext context(dxcOpts, dxcSupport);
      // TODO: implement all other actions.
      if (!dxcOpts.Preprocess.empty()) {
        context.Preprocess();
      } else if (dxcOpts.DumpBin) {
        retVal = context.DumpBinary();
      } else {
        retVal = context.Compile(path, bLinkLib);
      }
    } catch (const ::hlsl::Exception &hlslException) {
      PrintHlslException(hlslException, command, errorStream);
      retVal = 1;
    } catch (std::bad_alloc &) {
      errorStream << command << " failed - out of memory.\n";
      retVal = 1;
    } catch (...) {
      errorStream << command << " failed - unknown error.\n";
      retVal = 1;
    }
  }

  errorStream.flush();
  return retVal;
}

static void WriteBlobToFile(IDxcBlob *pBlob, llvm::StringRef FName,
                            UINT32 defaultTextCodePage) {
  ::dxc::WriteBlobToFile(pBlob, StringRefWide(FName), defaultTextCodePage);
}

static void WritePartToFile(IDxcBlob *pBlob, hlsl::DxilFourCC CC,
                            llvm::StringRef FName) {
  const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize());
  if (!pContainer) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }
  hlsl::DxilPartIsType pred(CC);
  hlsl::DxilPartIterator it =
      std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer), pred);
  if (it == hlsl::end(pContainer)) {
    throw hlsl::Exception(E_FAIL, "Unable to find required part in blob");
  }

  const char *pData = hlsl::GetDxilPartData(*it);
  DWORD dataLen = (*it)->PartSize;
  StringRefWide WideName(FName);
  CHandle file(CreateFileW(WideName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (file == INVALID_HANDLE_VALUE) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
  DWORD written;
  if (FALSE == WriteFile(file, pData, dataLen, &written, nullptr)) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), WideName);
  }
}

// This function is called either after the compilation is done or /dumpbin
// option is provided Performing options that are used to process dxil
// container.
int DxcContext::ActOnBlob(IDxcBlob *pBlob) {
  return ActOnBlob(pBlob, nullptr, nullptr);
}

int DxcContext::ActOnBlob(IDxcBlob *pBlob, IDxcBlob *pDebugBlob,
                          LPCWSTR pDebugBlobName) {
  int retVal = 0;
  // Text output.
  if (m_Opts.AstDump || m_Opts.OptDump) {
    WriteBlobToConsole(pBlob);
    return retVal;
  }

  // Write the output blob.
  if (!m_Opts.OutputObject.empty()) {
    // For backward compatability: fxc requires /Fo for /extractrootsignature
    if (!m_Opts.ExtractRootSignature) {
      CComPtr<IDxcBlob> pResult;
      UpdatePart(pBlob, &pResult);
      WriteBlobToFile(pResult, m_Opts.OutputObject, m_Opts.DefaultTextCodePage);
    }
  }

  // Verify Root Signature
  if (!m_Opts.VerifyRootSignatureSource.empty()) {
    return VerifyRootSignature();
  }

  // Extract and write the PDB/debug information.
  if (!m_Opts.DebugFile.empty()) {
    IFTBOOLMSG(m_Opts.DebugInfo, E_INVALIDARG,
               "/Fd specified, but no Debug Info was "
               "found in the shader, please use the "
               "/Zi switch to generate debug "
               "information compiling this shader.");

    if (pDebugBlob != nullptr) {
      IFTBOOLMSG(pDebugBlobName && *pDebugBlobName, E_INVALIDARG,
                 "/Fd was specified but no debug name was produced");
      WriteBlobToFile(pDebugBlob, pDebugBlobName, m_Opts.DefaultTextCodePage);
    } else {
      WritePartToFile(pBlob, hlsl::DFCC_ShaderDebugInfoDXIL, m_Opts.DebugFile);
    }
  }

  // Extract and write root signature information.
  if (m_Opts.ExtractRootSignature) {
    CComPtr<IDxcBlob> pRootSignatureContainer;
    ExtractRootSignature(pBlob, &pRootSignatureContainer);
    WriteBlobToFile(pRootSignatureContainer, m_Opts.OutputObject,
                    m_Opts.DefaultTextCodePage);
  }

  // Extract and write private data.
  if (!m_Opts.ExtractPrivateFile.empty()) {
    WritePartToFile(pBlob, hlsl::DFCC_PrivateData, m_Opts.ExtractPrivateFile);
  }

  // OutputObject suppresses console dump.
  bool needDisassembly =
      !m_Opts.OutputHeader.empty() || !m_Opts.AssemblyCode.empty() ||
      (m_Opts.OutputObject.empty() && m_Opts.DebugFile.empty() &&
       m_Opts.ExtractPrivateFile.empty() &&
       m_Opts.VerifyRootSignatureSource.empty() &&
       !m_Opts.ExtractRootSignature);

  if (!needDisassembly)
    return retVal;

  CComPtr<IDxcBlobEncoding> pDisassembleResult;

  if (m_Opts.IsRootSignatureProfile()) {
    // keep the same behavior as fxc, people may want to embed the root
    // signatures in their code bases.
    CComPtr<IDxcLibrary> pLibrary;
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    std::string Message = "Disassembly failed";
    IFT(pLibrary->CreateBlobWithEncodingOnHeapCopy(
        (LPBYTE)&Message[0], Message.size(), CP_ACP, &pDisassembleResult));
  } else {
    CComPtr<IDxcCompiler> pCompiler;
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    IFT(pCompiler->Disassemble(pBlob, &pDisassembleResult));
  }

  if (!m_Opts.OutputHeader.empty()) {
    llvm::Twine varName = m_Opts.VariableName.empty()
                              ? llvm::Twine("g_", m_Opts.EntryPoint)
                              : m_Opts.VariableName;
    WriteHeader(pDisassembleResult, pBlob, varName,
                StringRefWide(m_Opts.OutputHeader));
  } else if (!m_Opts.AssemblyCode.empty()) {
    WriteBlobToFile(pDisassembleResult, m_Opts.AssemblyCode,
                    m_Opts.DefaultTextCodePage);
  } else {
    WriteBlobToConsole(pDisassembleResult);
  }
  return retVal;
}

// Given a dxil container, update the dxil container by processing container
// specific options.
void DxcContext::UpdatePart(IDxcBlob *pSource, IDxcBlob **ppResult) {
  DXASSERT(pSource && ppResult, "otherwise blob cannot be updated");
  if (!UpdatePartRequired()) {
    *ppResult = pSource;
    pSource->AddRef();
    return;
  }

  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  CComPtr<IDxcBlob> pResult;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcContainerBuilder,
                                  &pContainerBuilder));

  // Load original container and update blob for each given option
  IFT(pContainerBuilder->Load(pSource));

  // Update parts based on dxc options
  if (m_Opts.StripDebug) {
    IFT(pContainerBuilder->RemovePart(
        hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL));
  }
  if (m_Opts.StripPrivate) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData));
  }
  if (m_Opts.StripRootSignature) {
    IFT(pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature));
  }
  if (!m_Opts.PrivateSource.empty()) {
    CComPtr<IDxcBlob> privateBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_PrivateData,
                                StringRefWide(m_Opts.PrivateSource),
                                &privateBlob));

    // setprivate option can replace existing private part.
    // Try removing the private data if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_PrivateData,
                                   privateBlob));
  }
  if (!m_Opts.RootSignatureSource.empty()) {
    // set rootsignature assumes that the given input is a dxil container.
    // We only want to add RTS0 part to the container builder.
    CComPtr<IDxcBlob> RootSignatureBlob;
    IFT(ReadFileIntoPartContent(hlsl::DxilFourCC::DFCC_RootSignature,
                                StringRefWide(m_Opts.RootSignatureSource),
                                &RootSignatureBlob));

    // setrootsignature option can replace existing rootsignature part
    // Try removing rootsignature if exists
    pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
    IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature,
                                   RootSignatureBlob));
  }

  // Get the final blob from container builder
  CComPtr<IDxcOperationResult> pBuilderResult;
  IFT(pContainerBuilder->SerializeContainer(&pBuilderResult));
  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pBuilderResult->GetErrorBuffer(&pErrors));
    if (pErrors != nullptr) {
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                      m_Opts.DefaultTextCodePage);
    }
  } else {
    WriteOperationErrorsToConsole(pBuilderResult, m_Opts.OutputWarnings);
  }
  HRESULT status;
  IFT(pBuilderResult->GetStatus(&status));
  IFT(status);
  IFT(pBuilderResult->GetResult(ppResult));
}

bool DxcContext::UpdatePartRequired() {
  return m_Opts.StripDebug || m_Opts.StripPrivate ||
         m_Opts.StripRootSignature || !m_Opts.PrivateSource.empty() ||
         !m_Opts.RootSignatureSource.empty();
}

// This function reads the file from input file and constructs a blob with
// fourCC parts Used for setprivate and setrootsignature option
HRESULT DxcContext::ReadFileIntoPartContent(hlsl::DxilFourCC fourCC,
                                            LPCWSTR fileName,
                                            IDxcBlob **ppResult) {
  DXASSERT(fourCC == hlsl::DxilFourCC::DFCC_PrivateData ||
               fourCC == hlsl::DxilFourCC::DFCC_RootSignature,
           "Otherwise we provided wrong part to read for updating part.");

  // Read result, if it's private data, then return the blob
  if (fourCC == hlsl::DxilFourCC::DFCC_PrivateData) {
    CComPtr<IDxcBlobEncoding> pResult;
    ReadFileIntoBlob(m_dxcSupport, fileName, &pResult);
    *ppResult = pResult.Detach();
  }

  // If root signature, check if it's a dxil container that contains
  // rootsignature part, then construct a blob of root signature part
  if (fourCC == hlsl::DxilFourCC::DFCC_RootSignature) {
    CComPtr<IDxcBlob> pResult;
    CComHeapPtr<BYTE> pData;
    DWORD dataSize;
    IFT(hlsl::ReadBinaryFile(fileName, (void **)&pData, &dataSize));
    DXASSERT(pData != nullptr,
             "otherwise ReadBinaryFile should throw an exception");
    hlsl::DxilContainerHeader *pHeader =
        hlsl::IsDxilContainerLike(pData.m_pData, dataSize);
    IFRBOOL(IsValidDxilContainer(pHeader, dataSize), E_INVALIDARG);
    hlsl::DxilPartHeader *pPartHeader =
        hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
    IFRBOOL(pPartHeader != nullptr, E_INVALIDARG);
    hlsl::DxcCreateBlobOnHeapCopy(hlsl::GetDxilPartData(pPartHeader),
                                  pPartHeader->PartSize, &pResult);
    *ppResult = pResult.Detach();
  }
  return S_OK;
}

// Constructs a dxil container builder with only root signature part.
// Right now IDxcContainerBuilder assumes that we are building a full dxil
// container, but we are building a container with only rootsignature part
void DxcContext::ExtractRootSignature(IDxcBlob *pBlob, IDxcBlob **ppResult) {

  DXASSERT_NOMSG(pBlob != nullptr && ppResult != nullptr);
  const hlsl::DxilContainerHeader *pHeader =
      (hlsl::DxilContainerHeader *)(pBlob->GetBufferPointer());
  IFTBOOL(hlsl::IsValidDxilContainer(pHeader, pHeader->ContainerSizeInBytes),
          DXC_E_CONTAINER_INVALID);
  const hlsl::DxilPartHeader *pPartHeader =
      hlsl::GetDxilPartByType(pHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  IFTBOOL(pPartHeader != nullptr, DXC_E_MISSING_PART);

  // Get new header and allocate memory for new container
  hlsl::DxilContainerHeader newHeader;
  uint32_t containerSize =
      hlsl::GetDxilContainerSizeFromParts(1, pPartHeader->PartSize);
  hlsl::InitDxilContainer(&newHeader, 1, containerSize);
  CComPtr<IMalloc> pMalloc;
  CComPtr<hlsl::AbstractMemoryStream> pMemoryStream;
  IFT(DxcCoGetMalloc(1, &pMalloc));
  IFT(hlsl::CreateMemoryStream(pMalloc, &pMemoryStream));
  ULONG cbWritten;

  // Write Container Header
  IFT(pMemoryStream->Write(&newHeader, sizeof(hlsl::DxilContainerHeader),
                           &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilContainerHeader), E_OUTOFMEMORY);

  // Write Part Offset
  uint32_t offset =
      sizeof(hlsl::DxilContainerHeader) + hlsl::GetOffsetTableSize(1);
  IFT(pMemoryStream->Write(&offset, sizeof(uint32_t), &cbWritten));
  IFTBOOL(cbWritten == sizeof(uint32_t), E_OUTOFMEMORY);

  // Write Root Signature Header
  IFT(pMemoryStream->Write(pPartHeader, sizeof(hlsl::DxilPartHeader),
                           &cbWritten));
  IFTBOOL(cbWritten == sizeof(hlsl::DxilPartHeader), E_OUTOFMEMORY);
  const char *partContent = hlsl::GetDxilPartData(pPartHeader);

  // Write Root Signature Content
  IFT(pMemoryStream->Write(partContent, pPartHeader->PartSize, &cbWritten));
  IFTBOOL(cbWritten == pPartHeader->PartSize, E_OUTOFMEMORY);

  // Return Result
  CComPtr<IDxcBlob> pResult;
  IFT(pMemoryStream->QueryInterface(&pResult));
  *ppResult = pResult.Detach();
}

int DxcContext::VerifyRootSignature() {
  // Get dxil container from file
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  hlsl::DxilContainerHeader *pSourceHeader =
      (hlsl::DxilContainerHeader *)pSource->GetBufferPointer();
  IFTBOOLMSG(hlsl::IsValidDxilContainer(pSourceHeader,
                                        pSourceHeader->ContainerSizeInBytes),
             E_INVALIDARG, "invalid DXIL container to verify.");

  // Get rootsignature from file
  CComPtr<IDxcBlob> pRootSignature;

  IFTMSG(ReadFileIntoPartContent(
             hlsl::DxilFourCC::DFCC_RootSignature,
             StringRefWide(m_Opts.VerifyRootSignatureSource), &pRootSignature),
         "invalid root signature to verify.");

  // TODO : Right now we are just going to bild a new blob with updated root
  // signature to verify root signature Since dxil container builder will verify
  // on its behalf. This does unnecessary memory allocation. We can improve this
  // later.
  CComPtr<IDxcContainerBuilder> pContainerBuilder;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcContainerBuilder,
                                  &pContainerBuilder));
  IFT(pContainerBuilder->Load(pSource));
  // Try removing root signature if it already exists
  pContainerBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature);
  IFT(pContainerBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature,
                                 pRootSignature));
  CComPtr<IDxcOperationResult> pOperationResult;
  IFT(pContainerBuilder->SerializeContainer(&pOperationResult));
  HRESULT status = E_FAIL;
  CComPtr<IDxcBlob> pResult;
  IFT(pOperationResult->GetStatus(&status));
  if (FAILED(status)) {
    if (!m_Opts.OutputWarningsFile.empty()) {
      CComPtr<IDxcBlobEncoding> pErrors;
      IFT(pOperationResult->GetErrorBuffer(&pErrors));
      WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                      m_Opts.DefaultTextCodePage);
    } else {
      WriteOperationErrorsToConsole(pOperationResult, m_Opts.OutputWarnings);
    }
    return 1;
  } else {
    printf("root signature verification succeeded.");
    return 0;
  }
}

class DxcIncludeHandlerForInjectedSources : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  DxcIncludeHandlerForInjectedSources() : m_dwRef(0){};
  std::unordered_map<std::wstring, CComPtr<IDxcBlob>> includeFiles;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  HRESULT insertIncludeFile(LPCWSTR pFilename, IDxcBlobEncoding *pBlob,
                            UINT32 dataLen) {
    try {
      includeFiles.try_emplace(std::wstring(pFilename), pBlob);
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename,
                                       IDxcBlob **ppIncludeSource) override {
    try {
      *ppIncludeSource = includeFiles.at(std::wstring(pFilename));
      (*ppIncludeSource)->AddRef();
    }
    CATCH_CPP_RETURN_HRESULT()
    return S_OK;
  }
};

int DxcContext::Compile(llvm::StringRef path, bool bLibLink) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pCompileResult;
  CComPtr<IDxcBlob> pDebugBlob;
  std::wstring debugName;
  {
    CComPtr<IDxcBlobEncoding> pSource;

    std::vector<std::wstring> argStrings;
    CopyArgsToWStrings(m_Opts.Args, CoreOption, argStrings);

    std::vector<LPCWSTR> args;
    args.reserve(argStrings.size());
    for (unsigned i = 0; i < argStrings.size(); i++) {
      const std::wstring &a = argStrings[i];
      if (a == L"-E" || a == L"-T") {
        // Skip entry and profile in arg.
        i++;
        continue;
      }
      args.push_back(a.data());
    }

    CComPtr<IDxcLibrary> pLibrary;
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    if (path.empty()) {
      ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
    } else {
      llvm::sys::fs::MSFileSystem *msfPtr;
      IFT(CreateMSFileSystemForDisk(&msfPtr));
      std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);

      ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
      IFTLLVM(pts.error_code());

      if (llvm::sys::fs::exists(m_Opts.InputFile)) {
        ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile),
                         &pSource);
      } else {
        SmallString<128> pathStr(path.begin(), path.end());
        llvm::sys::path::append(pathStr, m_Opts.InputFile.begin(),
                                m_Opts.InputFile.end());
        ReadFileIntoBlob(m_dxcSupport, StringRefWide(pathStr.str().str()),
                         &pSource);
      }
    }
    IFTARG(pSource->GetBufferSize() >= 4);

    CComPtr<IDxcIncludeHandler> pIncludeHandler;
    IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

    // Upgrade profile to 6.0 version from minimum recognized shader model
    llvm::StringRef TargetProfile = m_Opts.TargetProfile;
    const hlsl::ShaderModel *SM =
        hlsl::ShaderModel::GetByName(m_Opts.TargetProfile.str().c_str());
    if (SM->IsValid() && SM->GetMajor() < 6) {
      TargetProfile = hlsl::ShaderModel::Get(SM->GetKind(), 6, 0)->GetName();
    }

    if (bLibLink) {
      IFT(DxilD3DCompile2(pSource->GetBufferPointer(), pSource->GetBufferSize(),
                          m_Opts.InputFile.str().c_str(),
                          m_Opts.EntryPoint.str().c_str(),
                          TargetProfile.str().c_str(), m_Opts.Defines.data(),
                          m_Opts.Defines.size(), args.data(), args.size(),
                          pIncludeHandler, &pCompileResult));
    } else {
      IFT(pCompiler->Compile(
          pSource, StringRefWide(m_Opts.InputFile),
          StringRefWide(m_Opts.EntryPoint), StringRefWide(TargetProfile),
          args.data(), args.size(), m_Opts.Defines.data(),
          m_Opts.Defines.size(), pIncludeHandler, &pCompileResult));
    }
  }

  if (!m_Opts.OutputWarningsFile.empty()) {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pCompileResult->GetErrorBuffer(&pErrors));
    WriteBlobToFile(pErrors, m_Opts.OutputWarningsFile,
                    m_Opts.DefaultTextCodePage);
  } else {
    WriteOperationErrorsToConsole(pCompileResult, m_Opts.OutputWarnings);
  }

  HRESULT status;
  IFT(pCompileResult->GetStatus(&status));
  if (SUCCEEDED(status) || m_Opts.AstDump || m_Opts.OptDump) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pCompileResult->GetResult(&pProgram));
    pCompiler.Release();
    pCompileResult.Release();
    if (pProgram.p != nullptr) {
      ActOnBlob(pProgram.p, pDebugBlob, debugName.c_str());
    }
  }
  return status;
}

int DxcContext::DumpBinary() {
  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  return ActOnBlob(pSource.p);
}

void DxcContext::Preprocess() {
  DXASSERT(!m_Opts.Preprocess.empty(),
           "else option reading should have failed");
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pPreprocessResult;
  CComPtr<IDxcBlobEncoding> pSource;
  std::vector<LPCWSTR> args;

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcIncludeHandler> pIncludeHandler;
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));

  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  IFT(m_dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  IFT(pCompiler->Preprocess(pSource, StringRefWide(m_Opts.InputFile),
                            args.data(), args.size(), m_Opts.Defines.data(),
                            m_Opts.Defines.size(), pIncludeHandler,
                            &pPreprocessResult));
  WriteOperationErrorsToConsole(pPreprocessResult, m_Opts.OutputWarnings);

  HRESULT status;
  IFT(pPreprocessResult->GetStatus(&status));
  if (SUCCEEDED(status)) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pPreprocessResult->GetResult(&pProgram));
    WriteBlobToFile(pProgram, m_Opts.Preprocess, m_Opts.DefaultTextCodePage);
  }
}

static void WriteString(HANDLE hFile, LPCSTR value, LPCWSTR pFileName) {
  DWORD written;
  if (FALSE == WriteFile(hFile, value, strlen(value) * sizeof(value[0]),
                         &written, nullptr))
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
}

void DxcContext::WriteHeader(IDxcBlobEncoding *pDisassembly, IDxcBlob *pCode,
                             llvm::Twine &pVariableName, LPCWSTR pFileName) {
  CHandle file(CreateFileW(pFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (file == INVALID_HANDLE_VALUE) {
    IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
  }

  {
    std::string s;
    llvm::raw_string_ostream OS(s);
    // Note: with \r\n line endings, writing the disassembly could be a simple
    // WriteBlobToHandle with a prior and following WriteString for #ifs
    OS << "#if 0\r\n";
    const uint8_t *pBytes = (const uint8_t *)pDisassembly->GetBufferPointer();
    size_t len = pDisassembly->GetBufferSize();
    s.reserve(len + len * 0.1f); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (pBytes[i] == '\n')
        OS << '\r';
      OS << pBytes[i];
    }
    OS << "\r\n#endif\r\n";
    OS.flush();
    WriteString(file, s.c_str(), pFileName);
  }

  {
    std::string s;
    llvm::raw_string_ostream OS(s);
    OS << "\r\nconst unsigned char " << pVariableName << "[] = {";
    const uint8_t *pBytes = (const uint8_t *)pCode->GetBufferPointer();
    size_t len = pCode->GetBufferSize();
    s.reserve(100 + len * 6 + (len / 12) * 3); // rough estimate
    for (size_t i = 0; i < len; ++i) {
      if (i != 0)
        OS << ',';
      if ((i % 12) == 0)
        OS << "\r\n ";
      OS << " 0x";
      if (pBytes[i] < 0x10)
        OS << '0';
      OS.write_hex(pBytes[i]);
    }
    OS << "\r\n};\r\n";
    OS.flush();
    WriteString(file, s.c_str(), pFileName);
  }
}

class DxcBatchContext {
public:
  DxcBatchContext(DxcOpts &Opts, SpecificDllLoader &dxcSupport)
      : m_Opts(Opts), m_dxcSupport(dxcSupport) {}

  int BatchCompile(bool bMultiThread, bool bLibLink);

private:
  DxcOpts &m_Opts;
  SpecificDllLoader &m_dxcSupport;
};

int DxcBatchContext::BatchCompile(bool bMultiThread, bool bLibLink) {
  int retVal = 0;
  SmallString<128> path(m_Opts.InputFile.begin(), m_Opts.InputFile.end());
  llvm::sys::path::remove_filename(path);

  CComPtr<IDxcBlobEncoding> pSource;
  ReadFileIntoBlob(m_dxcSupport, StringRefWide(m_Opts.InputFile), &pSource);
  llvm::StringRef source((char *)pSource->GetBufferPointer(),
                         pSource->GetBufferSize());
  llvm::SmallVector<llvm::StringRef, 4> commands;
  source.split(commands, "\n", /*MaxSplit*/ -1, /*KeepEmpty*/ false);

  if (bMultiThread) {
    unsigned int threadNum = std::min<unsigned>(
        std::thread::hardware_concurrency(), commands.size());
    auto empty_fn = []() {};
    std::vector<std::thread> threads(threadNum);
    std::vector<std::string> errorStrings(threadNum);
    for (unsigned i = 0; i < threadNum; i++)
      threads[i] = std::thread(empty_fn);

    for (unsigned i = 0; i < commands.size(); i++) {
      // trim to remove /r if exist.
      llvm::StringRef command = commands[i].trim();
      if (command.empty())
        continue;
      if (command.startswith("//"))
        continue;

      unsigned threadIdx = i % threadNum;
      threads[threadIdx].join();

      threads[threadIdx] =
          std::thread(::Compile, command, std::ref(m_dxcSupport), path.str(),
                      bLibLink, std::ref(errorStrings[threadIdx]));
    }
    for (auto &th : threads)
      th.join();

    for (unsigned i = 0; i < threadNum; i++) {
      auto &errorString = errorStrings[i];
      if (errorString.size()) {
        fprintf(stderr, "dxc_batch failed : %s", errorString.c_str());
        if (0 == retVal)
          retVal = 1;
      }
    }
  } else {
    for (llvm::StringRef command : commands) {
      // trim to remove /r if exist.
      command = command.trim();
      if (command.empty())
        continue;
      if (command.startswith("//"))
        continue;
      std::string errorString;
      int ret =
          Compile(command, m_dxcSupport, path.str(), bLibLink, errorString);
      if (ret && 0 == retVal)
        retVal = ret;
      if (errorString.size()) {
        fprintf(stderr, "dxc_batch failed : %s", errorString.c_str());
        if (0 == retVal)
          retVal = 1;
      }
    }
  }
  return retVal;
}

int __cdecl wmain(int argc, const wchar_t **argv_) {
  const char *pStage = "Initialization";
  int retVal = 0;
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return 1;
  llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
  try {
    auto t_start = std::chrono::high_resolution_clock::now();

    std::error_code ec = hlsl::options::initHlslOptTable();
    if (ec) {
      fprintf(stderr, "%s failed - %s.\n", pStage, ec.message().c_str());
      return ec.value();
    }

    pStage = "Argument processing";
    const char *kMultiThreadArg = "-multi-thread";
    bool bMultiThread = false;
    const char *kLibLinkArg = "-lib-link";
    bool bLibLink = false;
    // Parse command line options.
    const OptTable *optionTable = getHlslOptTable();
    MainArgs argStrings(argc, argv_);
    llvm::ArrayRef<const char *> tmpArgStrings = argStrings.getArrayRef();
    std::vector<std::string> args(tmpArgStrings.begin(), tmpArgStrings.end());
    // Add target to avoid fail.
    args.emplace_back("-T");
    args.emplace_back("lib_6_x");

    std::vector<StringRef> refArgs;
    refArgs.reserve(args.size());
    for (auto &arg : args) {
      if (arg != kMultiThreadArg && arg != kLibLinkArg) {
        refArgs.emplace_back(arg.c_str());
      } else if (arg == kLibLinkArg) {
        bLibLink = true;
      } else {
        bMultiThread = true;
      }
    }

    MainArgs batchArgStrings(refArgs);

    DxcOpts dxcOpts;
    DXCLibraryDllLoader dxcSupport;

    // Read options and check errors.
    {
      std::string errorString;
      llvm::raw_string_ostream errorStream(errorString);
      int optResult = ReadDxcOpts(optionTable, DxcFlags, batchArgStrings,
                                  dxcOpts, errorStream);
      // TODO: validate unused option for dxc_bach.
      errorStream.flush();
      if (errorString.size()) {
        fprintf(stderr, "dxc_batch failed : %s", errorString.data());
      }
      if (optResult != 0) {
        return optResult;
      }
    }

    // Handle help request, which overrides any other processing.
    if (dxcOpts.ShowHelp) {
      std::string helpString;
      llvm::raw_string_ostream helpStream(helpString);
      optionTable->PrintHelp(helpStream, "dxc_batch.exe", "HLSL Compiler", "");
      helpStream << "multi-thread";
      helpStream.flush();
      dxc::WriteUtf8ToConsoleSizeT(helpString.data(), helpString.size());
      return 0;
    }

    // Setup a helper DLL.
    {
      std::string dllErrorString;
      llvm::raw_string_ostream dllErrorStream(dllErrorString);
      int dllResult =
          SetupSpecificDllLoader(dxcOpts, dxcSupport, dllErrorStream);
      dllErrorStream.flush();
      if (dllErrorString.size()) {
        fprintf(stderr, "%s", dllErrorString.data());
      }
      if (dllResult)
        return dllResult;
    }

    EnsureEnabled(dxcSupport);
    DxcBatchContext context(dxcOpts, dxcSupport);
    pStage = "BatchCompilation";
    retVal = context.BatchCompile(bMultiThread, bLibLink);
    {
      auto t_end = std::chrono::high_resolution_clock::now();
      double duration_ms =
          std::chrono::duration<double, std::milli>(t_end - t_start).count();

      fprintf(stderr, "duration: %f sec\n", duration_ms / 1000);
    }
  } catch (const ::hlsl::Exception &hlslException) {
    std::string errorString;
    llvm::raw_string_ostream errorStream(errorString);
    PrintHlslException(hlslException, pStage, errorStream);
    errorStream.flush();
    fprintf(stderr, "dxc_batch failed : %s", errorString.c_str());
    return 1;
  } catch (std::bad_alloc &) {
    printf("%s failed - out of memory.\n", pStage);
    return 1;
  } catch (...) {
    printf("%s failed - unknown error.\n", pStage);
    return 1;
  }
  return retVal;
}
