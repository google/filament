///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lib_cache_manager.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements lib blob cache manager.                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include "lib_share_helper.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/STLExtras.h"
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

using namespace llvm;
using namespace libshare;

#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxctools.h"
namespace {
class NoFuncBodyRewriter {
public:
  NoFuncBodyRewriter() {
    if (!m_dllSupport.IsEnabled())
      m_dllSupport.Initialize();
    m_dllSupport.CreateInstance(CLSID_DxcRewriter, &m_pRewriter);
  }
  HRESULT RewriteToNoFuncBody(LPCWSTR pFilename, IDxcBlobEncoding *pSource,
                              std::vector<DxcDefine> &m_defines,
                              IDxcBlob **ppNoFuncBodySource);

private:
  CComPtr<IDxcRewriter> m_pRewriter;
  dxc::DxCompilerDllLoader m_dllSupport;
};

HRESULT NoFuncBodyRewriter::RewriteToNoFuncBody(
    LPCWSTR pFilename, IDxcBlobEncoding *pSource,
    std::vector<DxcDefine> &m_defines, IDxcBlob **ppNoFuncBodySource) {
  // Create header with no function body.
  CComPtr<IDxcOperationResult> pRewriteResult;
  IFT(m_pRewriter->RewriteUnchangedWithInclude(
      pSource, pFilename, m_defines.data(), m_defines.size(),
      // Don't need include handler here, include already read in
      // RewriteIncludesToSnippet
      nullptr,
      RewriterOptionMask::SkipFunctionBody | RewriterOptionMask::KeepUserMacro,
      &pRewriteResult));
  HRESULT status;
  if (!SUCCEEDED(pRewriteResult->GetStatus(&status)) || !SUCCEEDED(status)) {
    CComPtr<IDxcBlobEncoding> pErr;
    IFT(pRewriteResult->GetErrorBuffer(&pErr));
    std::string errString =
        std::string((char *)pErr->GetBufferPointer(), pErr->GetBufferSize());
    IFTMSG(E_FAIL, errString);
    return E_FAIL;
  };

  // Get result.
  IFT(pRewriteResult->GetResult(ppNoFuncBodySource));
  return S_OK;
}
} // namespace

namespace {

struct KeyHash {
  std::size_t operator()(const hash_code &k) const { return k; }
};
struct KeyEqual {
  bool operator()(const hash_code &l, const hash_code &r) const {
    return l == r;
  }
};

class LibCacheManagerImpl : public libshare::LibCacheManager {
public:
  ~LibCacheManagerImpl() {}
  HRESULT AddLibBlob(std::string &processedHeader, const std::string &snippet,
                     CompileInput &compiler, size_t &hash,
                     IDxcBlob **pResultLib,
                     std::function<void(IDxcBlob *pSource)> compileFn) override;
  bool GetLibBlob(std::string &processedHeader, const std::string &snippet,
                  CompileInput &compiler, size_t &hash,
                  IDxcBlob **pResultLib) override;
  void Release() { m_libCache.clear(); }

private:
  hash_code GetHash(const std::string &header, const std::string &snippet,
                    CompileInput &compiler);
  using libCacheType =
      std::unordered_map<hash_code, CComPtr<IDxcBlob>, KeyHash, KeyEqual>;
  libCacheType m_libCache;
  using headerCacheType =
      std::unordered_map<hash_code, std::string, KeyHash, KeyEqual>;
  headerCacheType m_headerCache;
  std::shared_mutex m_mutex;
  NoFuncBodyRewriter m_rewriter;
};

static hash_code CombineWStr(hash_code hash, LPCWSTR Arg) {
  CW2A pUtf8Arg(Arg);
  unsigned length = strlen(pUtf8Arg.m_psz);
  return hash_combine(hash, StringRef(pUtf8Arg.m_psz, length));
}

hash_code LibCacheManagerImpl::GetHash(const std::string &header,
                                       const std::string &snippet,
                                       CompileInput &compiler) {
  hash_code libHash = hash_value(header);
  libHash = hash_combine(libHash, snippet);
  // Combine compile input.
  for (auto &Arg : compiler.arguments) {
    libHash = CombineWStr(libHash, Arg);
  }
  // snippet has been processed so don't add define to hash.
  return libHash;
}

using namespace hlsl;

HRESULT LibCacheManagerImpl::AddLibBlob(
    std::string &processedHeader, const std::string &snippet,
    CompileInput &compiler, size_t &hash, IDxcBlob **pResultLib,
    std::function<void(IDxcBlob *pSource)> compileFn) {
  if (!pResultLib) {
    return E_FAIL;
  }

  std::unique_lock<std::shared_mutex> lk(m_mutex);

  auto it = m_libCache.find(hash);
  if (it != m_libCache.end()) {
    *pResultLib = it->second;
    DXASSERT(m_headerCache.count(hash), "else mismatch header and lib");
    processedHeader = m_headerCache[hash];
    return S_OK;
  }
  std::string shader = processedHeader + snippet;
  CComPtr<IDxcBlobEncoding> pSource;
  IFT(DxcCreateBlobWithEncodingOnMallocCopy(
      GetGlobalHeapMalloc(), shader.data(), shader.size(), CP_UTF8, &pSource));

  compileFn(pSource);

  m_libCache[hash] = *pResultLib;

  // Rewrite curHeader to remove function body.
  CComPtr<IDxcBlob> result;
  IFT(m_rewriter.RewriteToNoFuncBody(L"input.hlsl", pSource, compiler.defines,
                                     &result));
  processedHeader = std::string((char *)(result)->GetBufferPointer(),
                                (result)->GetBufferSize());
  m_headerCache[hash] = processedHeader;
  return S_OK;
}

bool LibCacheManagerImpl::GetLibBlob(std::string &processedHeader,
                                     const std::string &snippet,
                                     CompileInput &compiler, size_t &hash,
                                     IDxcBlob **pResultLib) {
  if (!pResultLib) {
    return false;
  }
  // Create hash from source.
  hash_code libHash = GetHash(processedHeader, snippet, compiler);
  hash = libHash;
  // lock
  std::shared_lock<std::shared_mutex> lk(m_mutex);

  auto it = m_libCache.find(libHash);
  if (it != m_libCache.end()) {
    *pResultLib = it->second;
    DXASSERT(m_headerCache.count(libHash), "else mismatch header and lib");
    processedHeader = m_headerCache[libHash];
    return true;
  } else {
    return false;
  }
}

LibCacheManager *GetLibCacheManagerPtr(bool bFree) {
  static std::unique_ptr<LibCacheManagerImpl> g_LibCache =
      llvm::make_unique<LibCacheManagerImpl>();
  if (bFree)
    g_LibCache.reset();
  return g_LibCache.get();
}

} // namespace

LibCacheManager &LibCacheManager::GetLibCacheManager() {
  return *GetLibCacheManagerPtr(/*bFree*/ false);
}

void LibCacheManager::ReleaseLibCacheManager() {
  GetLibCacheManagerPtr(/*bFree*/ true);
}