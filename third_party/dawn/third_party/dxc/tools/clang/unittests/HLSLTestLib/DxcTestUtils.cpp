///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcTestUtils.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utility function implementations for testing dxc APIs                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HlslTestUtils.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Regex.h"

using namespace std;
using namespace hlsl_test;

MODULE_SETUP(TestModuleSetup)
MODULE_CLEANUP(TestModuleCleanup)

bool TestModuleSetup() {
  // Use this module-level function to set up LLVM dependencies.
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return false;
  if (FAILED(DxcInitThreadMalloc()))
    return false;
  DxcSetThreadMallocToDefault();

  if (hlsl::options::initHlslOptTable()) {
    return false;
  }
  return true;
}

bool TestModuleCleanup() {
  // Use this module-level function to set up LLVM dependencies.
  // In particular, clean up managed static allocations used by
  // parsing options with the LLVM library.
  ::hlsl::options::cleanupHlslOptTable();
  ::llvm::llvm_shutdown();
  DxcClearThreadMalloc();
  DxcCleanupThreadMalloc();

  // Make sure we can run init/cleanup mulitple times.
  if (FAILED(DxcInitThreadMalloc()))
    return false;
  DxcCleanupThreadMalloc();

  llvm::sys::fs::CleanupPerThreadFileSystem();
  return true;
}

std::shared_ptr<HlslIntellisenseSupport> CompilationResult::DefaultHlslSupport;

void CheckOperationSucceeded(IDxcOperationResult *pResult, IDxcBlob **ppBlob) {
  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
  VERIFY_SUCCEEDED(pResult->GetResult(ppBlob));
}

static bool CheckMsgs(llvm::StringRef text, llvm::ArrayRef<LPCSTR> pMsgs,
                      bool bRegex) {
  const char *pStart = !text.empty() ? text.begin() : nullptr;
  const char *pEnd = !text.empty() ? text.end() : nullptr;
  for (auto pMsg : pMsgs) {
    if (bRegex) {
      llvm::Regex RE(pMsg);
      std::string reErrors;
      VERIFY_IS_TRUE(RE.isValid(reErrors));
      if (!RE.match(text)) {
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"Unable to find regex '%S' in text:\r\n%.*S", pMsg,
            (pEnd - pStart), pStart));
        VERIFY_IS_TRUE(false);
      }
    } else {
      const char *pMatch = std::search(pStart, pEnd, pMsg, pMsg + strlen(pMsg));
      if (pEnd == pMatch) {
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"Unable to find '%S' in text:\r\n%.*S", pMsg, (pEnd - pStart),
            pStart));
      }
      VERIFY_IS_FALSE(pEnd == pMatch);
    }
  }
  return true;
}

bool CheckMsgs(const LPCSTR pText, size_t TextCount, const LPCSTR *pErrorMsgs,
               size_t errorMsgCount, bool bRegex) {
  return CheckMsgs(llvm::StringRef(pText, TextCount),
                   llvm::ArrayRef<LPCSTR>(pErrorMsgs, errorMsgCount), bRegex);
}

static bool CheckNotMsgs(llvm::StringRef text, llvm::ArrayRef<LPCSTR> pMsgs,
                         bool bRegex) {
  const char *pStart = !text.empty() ? text.begin() : nullptr;
  const char *pEnd = !text.empty() ? text.end() : nullptr;
  for (auto pMsg : pMsgs) {
    if (bRegex) {
      llvm::Regex RE(pMsg);
      std::string reErrors;
      VERIFY_IS_TRUE(RE.isValid(reErrors));
      if (RE.match(text)) {
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"Unexpectedly found regex '%S' in text:\r\n%.*S", pMsg,
            (pEnd - pStart), pStart));
        VERIFY_IS_TRUE(false);
      }
    } else {
      const char *pMatch = std::search(pStart, pEnd, pMsg, pMsg + strlen(pMsg));
      if (pEnd != pMatch) {
        WEX::Logging::Log::Comment(WEX::Common::String().Format(
            L"Unexpectedly found '%S' in text:\r\n%.*S", pMsg, (pEnd - pStart),
            pStart));
      }
      VERIFY_IS_TRUE(pEnd == pMatch);
    }
  }
  return true;
}

bool CheckNotMsgs(const LPCSTR pText, size_t TextCount,
                  const LPCSTR *pErrorMsgs, size_t errorMsgCount, bool bRegex) {
  return CheckNotMsgs(llvm::StringRef(pText, TextCount),
                      llvm::ArrayRef<LPCSTR>(pErrorMsgs, errorMsgCount),
                      bRegex);
}

bool CheckOperationResultMsgs(IDxcOperationResult *pResult,
                              llvm::ArrayRef<LPCSTR> pErrorMsgs,
                              bool maySucceedAnyway, bool bRegex) {
  HRESULT status;
  CComPtr<IDxcBlobEncoding> textBlob;
  if (!pResult)
    return true;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&textBlob));
  std::string textUtf8 = BlobToUtf8(textBlob);
  const char *pStart = !textUtf8.empty() ? textUtf8.c_str() : nullptr;
  const char *pEnd = !textUtf8.empty() ? pStart + textUtf8.length() : nullptr;
  if (pErrorMsgs.empty() || (pErrorMsgs.size() == 1 && !pErrorMsgs[0])) {
    if (FAILED(status) && pStart) {
      WEX::Logging::Log::Comment(WEX::Common::String().Format(
          L"Expected success but found errors\r\n%.*S", (pEnd - pStart),
          pStart));
    }
    VERIFY_SUCCEEDED(status);
  } else {
    if (SUCCEEDED(status) && maySucceedAnyway) {
      return false;
    }
    CheckMsgs(textUtf8, pErrorMsgs, bRegex);
  }
  return true;
}

bool CheckOperationResultMsgs(IDxcOperationResult *pResult,
                              const LPCSTR *pErrorMsgs, size_t errorMsgCount,
                              bool maySucceedAnyway, bool bRegex) {
  return CheckOperationResultMsgs(
      pResult, llvm::ArrayRef<LPCSTR>(pErrorMsgs, errorMsgCount),
      maySucceedAnyway, bRegex);
}

std::string DisassembleProgram(dxc::DxcDllSupport &dllSupport,
                               IDxcBlob *pProgram) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pDisassembly;

  if (!dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(dllSupport.Initialize());
  }

  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembly));
  return BlobToUtf8(pDisassembly);
}

void AssembleToContainer(dxc::DxcDllSupport &dllSupport, IDxcBlob *pModule,
                         IDxcBlob **pContainer) {
  CComPtr<IDxcAssembler> pAssembler;
  CComPtr<IDxcOperationResult> pResult;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
  VERIFY_SUCCEEDED(pAssembler->AssembleToContainer(pModule, &pResult));
  CheckOperationSucceeded(pResult, pContainer);
}

///////////////////////////////////////////////////////////////////////////////
// Helper functions to deal with passes.

void SplitPassList(LPWSTR pPassesBuffer, std::vector<LPCWSTR> &passes) {
  while (*pPassesBuffer) {
    // Skip comment lines.
    if (*pPassesBuffer == L'#') {
      while (*pPassesBuffer && *pPassesBuffer != '\n' &&
             *pPassesBuffer != '\r') {
        ++pPassesBuffer;
      }
      while (*pPassesBuffer == '\n' || *pPassesBuffer == '\r') {
        ++pPassesBuffer;
      }
      continue;
    }
    // Every other line is an option. Find the end of the line/buffer and
    // terminate it.
    passes.push_back(pPassesBuffer);
    while (*pPassesBuffer && *pPassesBuffer != '\n' && *pPassesBuffer != '\r') {
      ++pPassesBuffer;
    }
    while (*pPassesBuffer == '\n' || *pPassesBuffer == '\r') {
      *pPassesBuffer = L'\0';
      ++pPassesBuffer;
    }
  }
}

std::string BlobToUtf8(IDxcBlob *pBlob) {
  if (!pBlob)
    return std::string();
  CComPtr<IDxcBlobUtf8> pBlobUtf8;
  if (SUCCEEDED(pBlob->QueryInterface(&pBlobUtf8)))
    return std::string(pBlobUtf8->GetStringPointer(),
                       pBlobUtf8->GetStringLength());
  CComPtr<IDxcBlobEncoding> pBlobEncoding;
  IFT(pBlob->QueryInterface(&pBlobEncoding));
  // if (FAILED(pBlob->QueryInterface(&pBlobEncoding))) {
  //   // Assume it is already UTF-8
  //   return std::string((const char*)pBlob->GetBufferPointer(),
  //                      pBlob->GetBufferSize());
  // }
  BOOL known;
  UINT32 codePage;
  IFT(pBlobEncoding->GetEncoding(&known, &codePage));
  if (!known) {
    throw std::runtime_error("unknown codepage for blob.");
  }
  std::string result;
  if (codePage == DXC_CP_WIDE) {
    const wchar_t *text = (const wchar_t *)pBlob->GetBufferPointer();
    size_t length = pBlob->GetBufferSize() / 2;
    if (length >= 1 && text[length - 1] == L'\0')
      length -= 1; // Exclude null-terminator
    Unicode::WideToUTF8String(text, length, &result);
    return result;
  } else if (codePage == CP_UTF8) {
    const char *text = (const char *)pBlob->GetBufferPointer();
    size_t length = pBlob->GetBufferSize();
    if (length >= 1 && text[length - 1] == '\0')
      length -= 1; // Exclude null-terminator
    result.resize(length);
    memcpy(&result[0], text, length);
    return result;
  } else {
    throw std::runtime_error("Unsupported codepage.");
  }
}

std::wstring BlobToWide(IDxcBlob *pBlob) {
  if (!pBlob)
    return std::wstring();
  CComPtr<IDxcBlobWide> pBlobWide;
  if (SUCCEEDED(pBlob->QueryInterface(&pBlobWide)))
    return std::wstring(pBlobWide->GetStringPointer(),
                        pBlobWide->GetStringLength());
  CComPtr<IDxcBlobEncoding> pBlobEncoding;
  IFT(pBlob->QueryInterface(&pBlobEncoding));
  BOOL known;
  UINT32 codePage;
  IFT(pBlobEncoding->GetEncoding(&known, &codePage));
  if (!known) {
    throw std::runtime_error("unknown codepage for blob.");
  }
  std::wstring result;
  if (codePage == DXC_CP_WIDE) {
    const wchar_t *text = (const wchar_t *)pBlob->GetBufferPointer();
    size_t length = pBlob->GetBufferSize() / 2;
    if (length >= 1 && text[length - 1] == L'\0')
      length -= 1; // Exclude null-terminator
    result.resize(length);
    memcpy(&result[0], text, length);
    return result;
  } else if (codePage == CP_UTF8) {
    const char *text = (const char *)pBlob->GetBufferPointer();
    size_t length = pBlob->GetBufferSize();
    if (length >= 1 && text[length - 1] == '\0')
      length -= 1; // Exclude null-terminator
    Unicode::UTF8ToWideString(text, length, &result);
    return result;
  } else {
    throw std::runtime_error("Unsupported codepage.");
  }
}

void Utf8ToBlob(dxc::DxcDllSupport &dllSupport, const char *pVal,
                IDxcBlobEncoding **ppBlob) {
  CComPtr<IDxcLibrary> library;
  IFT(dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
  IFT(library->CreateBlobWithEncodingOnHeapCopy(pVal, strlen(pVal), CP_UTF8,
                                                ppBlob));
}

void MultiByteStringToBlob(dxc::DxcDllSupport &dllSupport,
                           const std::string &val, UINT32 codePage,
                           IDxcBlobEncoding **ppBlob) {
  CComPtr<IDxcLibrary> library;
  IFT(dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
  IFT(library->CreateBlobWithEncodingOnHeapCopy(val.data(), val.size(),
                                                codePage, ppBlob));
}

void MultiByteStringToBlob(dxc::DxcDllSupport &dllSupport,
                           const std::string &val, UINT32 codePage,
                           IDxcBlob **ppBlob) {
  MultiByteStringToBlob(dllSupport, val, codePage, (IDxcBlobEncoding **)ppBlob);
}

void Utf8ToBlob(dxc::DxcDllSupport &dllSupport, const std::string &val,
                IDxcBlobEncoding **ppBlob) {
  MultiByteStringToBlob(dllSupport, val, CP_UTF8, ppBlob);
}

void Utf8ToBlob(dxc::DxcDllSupport &dllSupport, const std::string &val,
                IDxcBlob **ppBlob) {
  Utf8ToBlob(dllSupport, val, (IDxcBlobEncoding **)ppBlob);
}

void WideToBlob(dxc::DxcDllSupport &dllSupport, const std::wstring &val,
                IDxcBlobEncoding **ppBlob) {
  CComPtr<IDxcLibrary> library;
  IFT(dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
  IFT(library->CreateBlobWithEncodingOnHeapCopy(
      val.data(), val.size() * sizeof(wchar_t), DXC_CP_WIDE, ppBlob));
}

void WideToBlob(dxc::DxcDllSupport &dllSupport, const std::wstring &val,
                IDxcBlob **ppBlob) {
  WideToBlob(dllSupport, val, (IDxcBlobEncoding **)ppBlob);
}

void VerifyCompileOK(dxc::DxcDllSupport &dllSupport, LPCSTR pText,
                     LPCWSTR pTargetProfile, LPCWSTR pArgs,
                     IDxcBlob **ppResult) {
  std::vector<std::wstring> argsW;
  std::vector<LPCWSTR> args;
  if (pArgs) {
    wistringstream argsS(pArgs);
    copy(istream_iterator<wstring, wchar_t>(argsS),
         istream_iterator<wstring, wchar_t>(), back_inserter(argsW));
    transform(argsW.begin(), argsW.end(), back_inserter(args),
              [](const wstring &w) { return w.data(); });
  }
  VerifyCompileOK(dllSupport, pText, pTargetProfile, args, ppResult);
}

void VerifyCompileOK(dxc::DxcDllSupport &dllSupport, LPCSTR pText,
                     LPCWSTR pTargetProfile, std::vector<LPCWSTR> &args,
                     IDxcBlob **ppResult) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pResult;
  HRESULT hrCompile;
  *ppResult = nullptr;
  VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  Utf8ToBlob(dllSupport, pText, &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      pTargetProfile, args.data(), args.size(),
                                      nullptr, 0, nullptr, &pResult));
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrCompile));
  VERIFY_SUCCEEDED(hrCompile);
  VERIFY_SUCCEEDED(pResult->GetResult(ppResult));
}

HRESULT GetVersion(dxc::DxcDllSupport &DllSupport, REFCLSID clsid,
                   unsigned &Major, unsigned &Minor) {
  CComPtr<IUnknown> pUnk;
  if (SUCCEEDED(DllSupport.CreateInstance(clsid, &pUnk))) {
    CComPtr<IDxcVersionInfo> pVersionInfo;
    IFR(pUnk.QueryInterface(&pVersionInfo));
    IFR(pVersionInfo->GetVersion(&Major, &Minor));
  }
  return S_OK;
}

bool ParseTargetProfile(llvm::StringRef targetProfile,
                        llvm::StringRef &outStage, unsigned &outMajor,
                        unsigned &outMinor) {
  auto stage_model = targetProfile.split("_");
  auto major_minor = stage_model.second.split("_");
  llvm::APInt major;
  if (major_minor.first.getAsInteger(16, major))
    return false;
  if (major_minor.second.compare("x") == 0) {
    outMinor = 0xF; // indicates offline target
  } else {
    llvm::APInt minor;
    if (major_minor.second.getAsInteger(16, minor))
      return false;
    outMinor = (unsigned)minor.getLimitedValue();
  }
  outStage = stage_model.first;
  outMajor = (unsigned)major.getLimitedValue();
  return true;
}

// VersionSupportInfo Implementation
VersionSupportInfo::VersionSupportInfo()
    : m_CompilerIsDebugBuild(false), m_InternalValidator(false), m_DxilMajor(0),
      m_DxilMinor(0), m_ValMajor(0), m_ValMinor(0) {}

void VersionSupportInfo::Initialize(dxc::DxcDllSupport &dllSupport) {
  VERIFY_IS_TRUE(dllSupport.IsEnabled());

  // Default to Dxil 1.0 and internal Val 1.0
  m_DxilMajor = m_ValMajor = 1;
  m_DxilMinor = m_ValMinor = 0;
  m_InternalValidator = true;
  CComPtr<IDxcVersionInfo> pVersionInfo;
  UINT32 VersionFlags = 0;

  // If the following fails, we have Dxil 1.0 compiler
  if (SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcCompiler, &pVersionInfo))) {
    VERIFY_SUCCEEDED(pVersionInfo->GetVersion(&m_DxilMajor, &m_DxilMinor));
    VERIFY_SUCCEEDED(pVersionInfo->GetFlags(&VersionFlags));
    m_CompilerIsDebugBuild =
        (VersionFlags & DxcVersionInfoFlags_Debug) ? true : false;
    pVersionInfo.Release();
  }

  if (SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcValidator, &pVersionInfo))) {
    VERIFY_SUCCEEDED(pVersionInfo->GetVersion(&m_ValMajor, &m_ValMinor));
    VERIFY_SUCCEEDED(pVersionInfo->GetFlags(&VersionFlags));
    if (m_ValMinor > 0) {
      // flag only exists on newer validator, assume internal otherwise.
      m_InternalValidator =
          (VersionFlags & DxcVersionInfoFlags_Internal) ? true : false;
    } else {
      // With old compiler, validator is the only way to get this
      m_CompilerIsDebugBuild =
          (VersionFlags & DxcVersionInfoFlags_Debug) ? true : false;
    }
  } else {
    // If create instance of IDxcVersionInfo on validator failed, we have an old
    // validator from dxil.dll
    m_InternalValidator = false;
  }
}
bool VersionSupportInfo::SkipIRSensitiveTest() {
  // Only debug builds preserve BB names.
  if (!m_CompilerIsDebugBuild) {
    WEX::Logging::Log::Comment(
        L"Test skipped due to name preservation requirement.");
    return true;
  }
  return false;
}
bool VersionSupportInfo::SkipDxilVersion(unsigned major, unsigned minor) {
  if (m_DxilMajor < major || (m_DxilMajor == major && m_DxilMinor < minor) ||
      m_ValMajor < major || (m_ValMajor == major && m_ValMinor < minor)) {
    WEX::Logging::Log::Comment(WEX::Common::String().Format(
        L"Test skipped because it requires Dxil %u.%u and Validator %u.%u.",
        major, minor, major, minor));
    return true;
  }
  return false;
}
bool VersionSupportInfo::SkipOutOfMemoryTest() { return false; }
