///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lib_share_preprocessor.cpp                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements preprocessor to split shader based on include.                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/microcom.h"

#include "dxc/Support/dxcfilesystem.h"
#include "lib_share_helper.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_set>

using namespace libshare;

using namespace hlsl;
using namespace llvm;

namespace dxcutil {
bool IsAbsoluteOrCurDirRelative(const Twine &T) {
  if (llvm::sys::path::is_absolute(T)) {
    return true;
  }
  if (T.isSingleStringRef()) {
    StringRef r = T.getSingleStringRef();
    if (r.size() < 2)
      return false;
    const char *pData = r.data();
    return pData[0] == '.' && (pData[1] == '\\' || pData[1] == '/');
  }
  DXASSERT(false, "twine kind not supported");
  return false;
}
} // namespace dxcutil

namespace {

const StringRef file_region = "\n#pragma region\n";

class IncPathIncludeHandler : public IDxcIncludeHandler {
public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  virtual ~IncPathIncludeHandler() {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<::IDxcIncludeHandler>(this, riid, ppvObject);
  }
  IncPathIncludeHandler(IDxcIncludeHandler *handler,
                        std::vector<std::string> &includePathList)
      : m_dwRef(0), m_pIncludeHandler(handler),
        m_includePathList(includePathList) {}
  HRESULT STDMETHODCALLTYPE LoadSource(
      LPCWSTR pFilename,         // Candidate filename.
      IDxcBlob **ppIncludeSource // Resultant
                                 // source object
                                 // for included
                                 // file, nullptr
                                 // if not found.
      ) override {
    CW2A pUtf8Filename(pFilename);
    if (m_loadedFileNames.find(pUtf8Filename.m_psz) !=
        m_loadedFileNames.end()) {
      // Already include this file.
      // Just return empty content.
      static const char kEmptyStr[] = " ";
      CComPtr<IDxcBlobEncoding> pEncodingIncludeSource;
      IFT(DxcCreateBlobWithEncodingOnMalloc(kEmptyStr, GetGlobalHeapMalloc(), 1,
                                            CP_UTF8, &pEncodingIncludeSource));
      *ppIncludeSource = pEncodingIncludeSource.Detach();
      return S_OK;
    }
    // Read file.
    CComPtr<IDxcBlob> pIncludeSource;
    if (m_includePathList.empty()) {
      IFT(m_pIncludeHandler->LoadSource(pFilename, ppIncludeSource));
    } else {
      bool bLoaded = false;
      // Not support same filename in different directory.
      for (std::string &path : m_includePathList) {
        std::string tmpFilenameStr =
            path + StringRef(pUtf8Filename.m_psz).str();
        CA2W pWTmpFilename(tmpFilenameStr.c_str());
        if (S_OK == m_pIncludeHandler->LoadSource(pWTmpFilename.m_psz,
                                                  ppIncludeSource)) {
          bLoaded = true;
          break;
        }
      }
      if (!bLoaded) {
        IFT(m_pIncludeHandler->LoadSource(pFilename, ppIncludeSource));
      }
    }
    CComPtr<IDxcBlobUtf8> utf8Source;
    IFT(hlsl::DxcGetBlobAsUtf8(*ppIncludeSource, DxcGetThreadMallocNoRef(),
                               &utf8Source));

    StringRef Data(utf8Source->GetStringPointer(),
                   utf8Source->GetStringLength());
    std::string strRegionData = (Twine(file_region) + Data + file_region).str();

    CComPtr<IDxcBlobEncoding> pEncodingIncludeSource;
    IFT(DxcCreateBlobWithEncodingOnMallocCopy(
        GetGlobalHeapMalloc(), strRegionData.c_str(), strRegionData.size(),
        CP_UTF8, &pEncodingIncludeSource));
    *ppIncludeSource = pEncodingIncludeSource.Detach();
    m_loadedFileNames.insert(pUtf8Filename.m_psz);
    return S_OK;
  }

private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)
  IDxcIncludeHandler *m_pIncludeHandler;
  std::unordered_set<std::string> m_loadedFileNames;
  std::vector<std::string> &m_includePathList;
};

} // namespace

HRESULT CreateCompiler(IDxcCompiler **ppCompiler);

namespace {

class IncludeToLibPreprocessorImpl : public IncludeToLibPreprocessor {
public:
  virtual ~IncludeToLibPreprocessorImpl() {}

  IncludeToLibPreprocessorImpl(IDxcIncludeHandler *handler)
      : m_pIncludeHandler(handler) {}

  void AddIncPath(StringRef path) override;
  HRESULT Preprocess(IDxcBlob *pSource, LPCWSTR pFilename, LPCWSTR *pArguments,
                     UINT32 argCount, const DxcDefine *pDefines,
                     unsigned defineCount) override;

  const std::vector<std::string> &GetSnippets() const override {
    return m_snippets;
  }

private:
  HRESULT SplitShaderIntoSnippets(IDxcBlob *pSource);
  IDxcIncludeHandler *m_pIncludeHandler;
  // Snippets split by #include.
  std::vector<std::string> m_snippets;
  std::vector<std::string> m_includePathList;
};

HRESULT
IncludeToLibPreprocessorImpl::SplitShaderIntoSnippets(IDxcBlob *pSource) {
  CComPtr<IDxcBlobUtf8> utf8Source;
  IFT(hlsl::DxcGetBlobAsUtf8(pSource, DxcGetThreadMallocNoRef(), &utf8Source));

  StringRef Data(utf8Source->GetStringPointer(), utf8Source->GetStringLength());
  SmallVector<StringRef, 8> splitResult;
  Data.split(splitResult, file_region, /*maxSplit*/ -1, /*keepEmpty*/ false);
  for (StringRef snippet : splitResult) {
    m_snippets.emplace_back(snippet);
  }
  return S_OK;
}

void IncludeToLibPreprocessorImpl::AddIncPath(StringRef path) {
  m_includePathList.emplace_back(path);
}

HRESULT IncludeToLibPreprocessorImpl::Preprocess(
    IDxcBlob *pSource, LPCWSTR pFilename, LPCWSTR *pArguments, UINT32 argCount,
    const DxcDefine *pDefines, unsigned defineCount) {
  std::string s, warnings;
  raw_string_ostream o(s);
  raw_string_ostream w(warnings);

  CW2A pUtf8Name(pFilename);

  IncPathIncludeHandler incPathIncludeHandler(m_pIncludeHandler,
                                              m_includePathList);
  // AddRef to hold incPathIncludeHandler.
  // If not, DxcArgsFileSystem will kill it.
  incPathIncludeHandler.AddRef();

  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcOperationResult> pPreprocessResult;

  IFR(CreateCompiler(&compiler));

  IFR(compiler->Preprocess(pSource, L"input.hlsl", pArguments, argCount,
                           pDefines, defineCount, &incPathIncludeHandler,
                           &pPreprocessResult));

  HRESULT status;
  IFT(pPreprocessResult->GetStatus(&status));
  if (SUCCEEDED(status)) {
    CComPtr<IDxcBlob> pProgram;
    IFT(pPreprocessResult->GetResult(&pProgram));
    IFT(SplitShaderIntoSnippets(pProgram));
  }
  return S_OK;
}
} // namespace

std::unique_ptr<IncludeToLibPreprocessor>
IncludeToLibPreprocessor::CreateIncludeToLibPreprocessor(
    IDxcIncludeHandler *handler) {
  return llvm::make_unique<IncludeToLibPreprocessorImpl>(handler);
}
