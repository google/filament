///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lib_share_compile.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Compile function split shader to lib then link.                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"

#include "dxc/Support/FileIOHelper.h"

#include "dxc/Support/Unicode.h"
#include "lib_share_helper.h"
#include <d3dcompiler.h>
#include <string>
#include <vector>

using namespace libshare;
using namespace hlsl;
using namespace llvm;

HRESULT CreateLibrary(IDxcLibrary **pLibrary) {
  return DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary),
                           (void **)pLibrary);
}

HRESULT CreateCompiler(IDxcCompiler **ppCompiler) {
  return DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
                           (void **)ppCompiler);
}

HRESULT CreateLinker(IDxcLinker **ppLinker) {
  return DxcCreateInstance(CLSID_DxcLinker, __uuidof(IDxcLinker),
                           (void **)ppLinker);
}

HRESULT CreateContainerReflection(IDxcContainerReflection **ppReflection) {
  return DxcCreateInstance(CLSID_DxcContainerReflection,
                           __uuidof(IDxcContainerReflection),
                           (void **)ppReflection);
}

HRESULT CompileToLib(IDxcBlob *pSource, std::vector<DxcDefine> &defines,
                     IDxcIncludeHandler *pInclude,
                     std::vector<LPCWSTR> &arguments, IDxcBlob **ppCode,
                     IDxcBlobEncoding **ppErrorMsgs) {
  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcOperationResult> operationResult;

  IFR(CreateCompiler(&compiler));
  IFR(compiler->Compile(pSource, L"input.hlsl", L"", L"lib_6_x",
                        arguments.data(), (UINT)arguments.size(),
                        defines.data(), (UINT)defines.size(), pInclude,
                        &operationResult));
  HRESULT hr;
  operationResult->GetStatus(&hr);
  if (SUCCEEDED(hr)) {
    return operationResult->GetResult(ppCode);
  } else {
    if (ppErrorMsgs)
      operationResult->GetErrorBuffer(ppErrorMsgs);
    return hr;
  }
  return hr;
}

#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/dxcapi.impl.h"
static void ReadOptsAndValidate(hlsl::options::MainArgs &mainArgs,
                                hlsl::options::DxcOpts &opts,
                                AbstractMemoryStream *pOutputStream,
                                IDxcOperationResult **ppResult,
                                bool &finished) {
  const llvm::opt::OptTable *table = ::options::getHlslOptTable();
  raw_stream_ostream outStream(pOutputStream);
  if (0 != hlsl::options::ReadDxcOpts(table, hlsl::options::CompilerFlags,
                                      mainArgs, opts, outStream)) {
    CComPtr<IDxcBlob> pErrorBlob;
    IFT(pOutputStream->QueryInterface(&pErrorBlob));
    outStream.flush();
    IFT(DxcResult::Create(
        E_INVALIDARG, DXC_OUT_NONE,
        {DxcOutputObject::ErrorOutput(opts.DefaultTextCodePage,
                                      (LPCSTR)pErrorBlob->GetBufferPointer(),
                                      pErrorBlob->GetBufferSize())},
        ppResult));
    finished = true;
    return;
  }
  DXASSERT(opts.HLSLVersion > hlsl::LangStd::v2015,
           "else ReadDxcOpts didn't fail for non-isense");
  finished = false;
}

HRESULT CompileFromBlob(IDxcBlobEncoding *pSource, LPCWSTR pSourceName,
                        std::vector<DxcDefine> &defines,
                        IDxcIncludeHandler *pInclude, LPCSTR pEntrypoint,
                        LPCSTR pTarget, std::vector<LPCWSTR> &arguments,
                        IDxcOperationResult **ppOperationResult) {
  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcLinker> linker;

  // Upconvert legacy targets
  const hlsl::ShaderModel *SM = hlsl::ShaderModel::GetByName(pTarget);
  const char *Target = pTarget;
  if (SM->IsValid() && SM->GetMajor() < 6) {
    Target = hlsl::ShaderModel::Get(SM->GetKind(), 6, 0)->GetName();
  }

  HRESULT hr = S_OK;
  try {
    CA2W pEntrypointW(pEntrypoint);
    CA2W pTargetProfileW(Target);

    // Preprocess.
    std::unique_ptr<IncludeToLibPreprocessor> preprocessor =
        IncludeToLibPreprocessor::CreateIncludeToLibPreprocessor(pInclude);

    if (arguments.size()) {
      CComPtr<AbstractMemoryStream> pOutputStream;
      IFT(CreateMemoryStream(GetGlobalHeapMalloc(), &pOutputStream));

      hlsl::options::MainArgs mainArgs(arguments.size(), arguments.data(), 0);
      hlsl::options::DxcOpts opts;
      bool finished;
      ReadOptsAndValidate(mainArgs, opts, pOutputStream, ppOperationResult,
                          finished);
      if (finished) {
        return E_FAIL;
      }

      for (const llvm::opt::Arg *A : opts.Args.filtered(options::OPT_I)) {
        if (dxcutil::IsAbsoluteOrCurDirRelative(A->getValue())) {
          preprocessor->AddIncPath(A->getValue());
        } else {
          std::string s("./");
          s += A->getValue();
          preprocessor->AddIncPath(s);
        }
      }
    }

    preprocessor->Preprocess(pSource, pSourceName, arguments.data(),
                             arguments.size(), defines.data(), defines.size());

    CompileInput compilerInput{defines, arguments};

    LibCacheManager &libCache = LibCacheManager::GetLibCacheManager();
    IFR(CreateLinker(&linker));
    IDxcIncludeHandler *const kNoIncHandler = nullptr;
    const auto &snippets = preprocessor->GetSnippets();
    std::string processedHeader = "";
    std::vector<std::wstring> hashStrList;
    std::vector<LPCWSTR> hashList;
// #define LIB_SHARE_DBG
#ifdef LIB_SHARE_DBG
    std::vector<std::wstring> defineList;
    defineList.emplace_back(L"");
    for (auto &def : defines) {
      std::wstring strDef = std::wstring(L"#define ") + std::wstring(def.Name);
      if (def.Value) {
        strDef.push_back(L' ');
        strDef += std::wstring(def.Value);
        strDef.push_back(L'\n');
      }
      defineList[0] += strDef;
      defineList.emplace_back(strDef);
    }
    std::string contents;
    std::vector<std::wstring> contentStrList;
    std::vector<LPCWSTR> contentList;
#endif
    for (const auto &snippet : snippets) {
      CComPtr<IDxcBlob> pOutputBlob;
      size_t hash;
#ifdef LIB_SHARE_DBG
      contents = processedHeader + snippet;
      CA2W tmpContents(contents.c_str());
      contentStrList.emplace_back(tmpContents.m_psz);
      contentList.emplace_back(contentStrList.back().c_str());
#endif
      if (!libCache.GetLibBlob(processedHeader, snippet, compilerInput, hash,
                               &pOutputBlob)) {
        // Cannot find existing blob, create from pSource.
        IDxcBlob **ppCode = &pOutputBlob;

        auto compileFn = [&](IDxcBlob *pSource) {
          IFT(CompileToLib(pSource, defines, kNoIncHandler, arguments, ppCode,
                           nullptr));
        };
        libCache.AddLibBlob(processedHeader, snippet, compilerInput, hash,
                            &pOutputBlob, compileFn);
      }
      hashStrList.emplace_back(std::to_wstring(hash));
      hashList.emplace_back(hashStrList.back().c_str());
      linker->RegisterLibrary(hashList.back(), pOutputBlob);
      pOutputBlob.Detach(); // Ownership is in libCache.
    }
    std::wstring wEntry = Unicode::UTF8ToWideStringOrThrow(pEntrypoint);
    std::wstring wTarget = Unicode::UTF8ToWideStringOrThrow(Target);

    // Link
#ifdef LIB_SHARE_DBG
    return linker->Link(wEntry.c_str(), wTarget.c_str(), hashList.data(),
                        hashList.size(), contentList.data(), 0,
                        ppOperationResult);
#else
    return linker->Link(wEntry.c_str(), wTarget.c_str(), hashList.data(),
                        hashList.size(), nullptr, 0, ppOperationResult);
#endif
  }
  CATCH_CPP_ASSIGN_HRESULT();
  return hr;
}

HRESULT WINAPI DxilD3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize,
                              LPCSTR pSourceName,
                              const D3D_SHADER_MACRO *pDefines,
                              IDxcIncludeHandler *pInclude, LPCSTR pEntrypoint,
                              LPCSTR pTarget, UINT Flags1, UINT Flags2,
                              ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcOperationResult> operationResult;
  *ppCode = nullptr;
  if (ppErrorMsgs != nullptr)
    *ppErrorMsgs = nullptr;

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingFromPinned(pSrcData, SrcDataSize, CP_ACP,
                                                &source));
  HRESULT hr = S_OK;
  CComPtr<IMalloc> m_pMalloc(GetGlobalHeapMalloc());
  DxcThreadMalloc TM(m_pMalloc);

  try {
    CA2W pFileName(pSourceName);

    std::vector<std::wstring> defineValues;
    std::vector<DxcDefine> defines;
    if (pDefines) {
      CONST D3D_SHADER_MACRO *pCursor = pDefines;

      // Convert to UTF-16.
      while (pCursor->Name) {
        defineValues.push_back(std::wstring(CA2W(pCursor->Name)));
        if (pCursor->Definition)
          defineValues.push_back(std::wstring(CA2W(pCursor->Definition)));
        else
          defineValues.push_back(std::wstring());
        ++pCursor;
      }

      // Build up array.
      pCursor = pDefines;
      size_t i = 0;
      while (pCursor->Name) {
        defines.push_back(
            DxcDefine{defineValues[i++].c_str(), defineValues[i++].c_str()});
        ++pCursor;
      }
    }

    std::vector<LPCWSTR> arguments;
    if (Flags1 & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)
      arguments.push_back(L"/Gec");
    // /Ges Not implemented:
    // if (Flags1 & D3DCOMPILE_ENABLE_STRICTNESS)
    // arguments.push_back(L"/Ges");
    if (Flags1 & D3DCOMPILE_IEEE_STRICTNESS)
      arguments.push_back(L"/Gis");
    if (Flags1 & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
      switch (Flags1 & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
      case D3DCOMPILE_OPTIMIZATION_LEVEL0:
        arguments.push_back(L"/O0");
        break;
      case D3DCOMPILE_OPTIMIZATION_LEVEL2:
        arguments.push_back(L"/O2");
        break;
      case D3DCOMPILE_OPTIMIZATION_LEVEL3:
        arguments.push_back(L"/O3");
        break;
      }
    }
    // Currently, /Od turns off too many optimization passes, causing incorrect
    // DXIL to be generated. Re-enable once /Od is implemented properly:
    // if(Flags1 & D3DCOMPILE_SKIP_OPTIMIZATION) arguments.push_back(L"/Od");
    if (Flags1 & D3DCOMPILE_DEBUG)
      arguments.push_back(L"/Zi");
    if (Flags1 & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR)
      arguments.push_back(L"/Zpr");
    if (Flags1 & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR)
      arguments.push_back(L"/Zpc");
    if (Flags1 & D3DCOMPILE_AVOID_FLOW_CONTROL)
      arguments.push_back(L"/Gfa");
    if (Flags1 & D3DCOMPILE_PREFER_FLOW_CONTROL)
      arguments.push_back(L"/Gfp");
    // We don't implement this:
    // if(Flags1 & D3DCOMPILE_PARTIAL_PRECISION) arguments.push_back(L"/Gpp");
    if (Flags1 & D3DCOMPILE_RESOURCES_MAY_ALIAS)
      arguments.push_back(L"/res_may_alias");

    CompileFromBlob(source, pFileName, defines, pInclude, pEntrypoint, pTarget,
                    arguments, &operationResult);
    operationResult->GetStatus(&hr);
    if (SUCCEEDED(hr)) {
      return operationResult->GetResult((IDxcBlob **)ppCode);
    } else {
      if (ppErrorMsgs)
        operationResult->GetErrorBuffer((IDxcBlobEncoding **)ppErrorMsgs);
      return hr;
    }
  }
  CATCH_CPP_ASSIGN_HRESULT();
  return hr;
}

HRESULT WINAPI DxilD3DCompile2(
    LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
    LPCSTR pEntrypoint, LPCSTR pTarget,
    const DxcDefine *pDefines, // Array of defines
    UINT32 defineCount,        // Number of defines
    LPCWSTR *pArguments,       // Array of pointers to arguments
    UINT32 argCount,           // Number of arguments
    IDxcIncludeHandler *pInclude, IDxcOperationResult **ppOperationResult) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;

  *ppOperationResult = nullptr;

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingFromPinned(pSrcData, SrcDataSize, CP_ACP,
                                                &source));
  HRESULT hr = S_OK;
  CComPtr<IMalloc> m_pMalloc(GetGlobalHeapMalloc());
  DxcThreadMalloc TM(m_pMalloc);
  try {
    CA2W pFileName(pSourceName);
    std::vector<DxcDefine> defines(pDefines, pDefines + defineCount);
    std::vector<LPCWSTR> arguments(pArguments, pArguments + argCount);
    return CompileFromBlob(source, pFileName, defines, pInclude, pEntrypoint,
                           pTarget, arguments, ppOperationResult);
  }
  CATCH_CPP_ASSIGN_HRESULT();
  return hr;
}
