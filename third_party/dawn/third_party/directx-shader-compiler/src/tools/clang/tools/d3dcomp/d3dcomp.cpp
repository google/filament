///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// d3dcomp.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides functions to bridge from d3dcompiler_47 to dxcompiler.           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"

#include <d3dcompiler.h>
#include <string>
#include <vector>

HRESULT CreateLibrary(IDxcLibrary **pLibrary) {
  return DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary),
                           (void **)pLibrary);
}

HRESULT CreateCompiler(IDxcCompiler **ppCompiler) {
  return DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
                           (void **)ppCompiler);
}

HRESULT CreateContainerReflection(IDxcContainerReflection **ppReflection) {
  return DxcCreateInstance(CLSID_DxcContainerReflection,
                           __uuidof(IDxcContainerReflection),
                           (void **)ppReflection);
}

HRESULT CompileFromBlob(IDxcBlobEncoding *pSource, LPCWSTR pSourceName,
                        const D3D_SHADER_MACRO *pDefines,
                        IDxcIncludeHandler *pInclude, LPCSTR pEntrypoint,
                        LPCSTR pTarget, UINT Flags1, UINT Flags2,
                        ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcOperationResult> operationResult;
  HRESULT hr;

  // Upconvert legacy targets
  char Target[7] = "?s_6_0";
  Target[6] = 0;
  if (pTarget[3] < '6') {
    Target[0] = pTarget[0];
    pTarget = Target;
  }

  try {
    CA2W pEntrypointW(pEntrypoint);
    CA2W pTargetProfileW(pTarget);
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
    // if(Flags1 & D3DCOMPILE_ENABLE_STRICTNESS) arguments.push_back(L"/Ges");
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
    arguments.push_back(L"-HV");
    arguments.push_back(L"2016");

    IFR(CreateCompiler(&compiler));
    IFR(compiler->Compile(pSource, pSourceName, pEntrypointW, pTargetProfileW,
                          arguments.data(), (UINT)arguments.size(),
                          defines.data(), (UINT)defines.size(), pInclude,
                          &operationResult));
  } catch (const std::bad_alloc &) {
    return E_OUTOFMEMORY;
  } catch (const CAtlException &err) {
    return err.m_hr;
  }

  operationResult->GetStatus(&hr);
  if (SUCCEEDED(hr)) {
    return operationResult->GetResult((IDxcBlob **)ppCode);
  } else {
    if (ppErrorMsgs)
      operationResult->GetErrorBuffer((IDxcBlobEncoding **)ppErrorMsgs);
    return hr;
  }
}

HRESULT WINAPI BridgeD3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize,
                                LPCSTR pSourceName,
                                const D3D_SHADER_MACRO *pDefines,
                                ID3DInclude *pInclude, LPCSTR pEntrypoint,
                                LPCSTR pTarget, UINT Flags1, UINT Flags2,
                                ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcIncludeHandler> includeHandler;

  *ppCode = nullptr;
  if (ppErrorMsgs != nullptr)
    *ppErrorMsgs = nullptr;

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingFromPinned(pSrcData, SrcDataSize, CP_ACP,
                                                &source));

  // Until we actually wrap the include handler, fail if there's a user-supplied
  // handler.
  if (D3D_COMPILE_STANDARD_FILE_INCLUDE == pInclude) {
    IFR(library->CreateIncludeHandler(&includeHandler));
  } else if (pInclude) {
    return E_INVALIDARG;
  }

  try {
    CA2W pFileName(pSourceName);
    return CompileFromBlob(source, pFileName, pDefines, includeHandler,
                           pEntrypoint, pTarget, Flags1, Flags2, ppCode,
                           ppErrorMsgs);
  } catch (const std::bad_alloc &) {
    return E_OUTOFMEMORY;
  } catch (const CAtlException &err) {
    return err.m_hr;
  }
}

HRESULT WINAPI BridgeD3DCompile2(
    LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
    const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint,
    LPCSTR pTarget, UINT Flags1, UINT Flags2, UINT SecondaryDataFlags,
    LPCVOID pSecondaryData, SIZE_T SecondaryDataSize, ID3DBlob **ppCode,
    ID3DBlob **ppErrorMsgs) {
  if (SecondaryDataFlags == 0 || pSecondaryData == nullptr) {
    return BridgeD3DCompile(pSrcData, SrcDataSize, pSourceName, pDefines,
                            pInclude, pEntrypoint, pTarget, Flags1, Flags2,
                            ppCode, ppErrorMsgs);
  }
  return E_NOTIMPL;
}

HRESULT WINAPI BridgeD3DCompileFromFile(
    LPCWSTR pFileName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude,
    LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2,
    ID3DBlob **ppCode, ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcIncludeHandler> includeHandler;
  HRESULT hr;

  *ppCode = nullptr;
  if (ppErrorMsgs != nullptr)
    *ppErrorMsgs = nullptr;

  hr = CreateLibrary(&library);
  if (FAILED(hr))
    return hr;
  hr = library->CreateBlobFromFile(pFileName, nullptr, &source);
  if (FAILED(hr))
    return hr;

  // Until we actually wrap the include handler, fail if there's a user-supplied
  // handler.
  if (D3D_COMPILE_STANDARD_FILE_INCLUDE == pInclude) {
    IFT(library->CreateIncludeHandler(&includeHandler));
  } else if (pInclude) {
    return E_INVALIDARG;
  }

  return CompileFromBlob(source, pFileName, pDefines, includeHandler,
                         pEntrypoint, pTarget, Flags1, Flags2, ppCode,
                         ppErrorMsgs);
}

HRESULT WINAPI BridgeD3DDisassemble(LPCVOID pSrcData, SIZE_T SrcDataSize,
                                    UINT Flags, LPCSTR szComments,
                                    ID3DBlob **ppDisassembly) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcBlobEncoding> disassemblyText;

  *ppDisassembly = nullptr;

  UNREFERENCED_PARAMETER(szComments);
  UNREFERENCED_PARAMETER(Flags);

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingFromPinned(pSrcData, SrcDataSize, CP_ACP,
                                                &source));
  IFR(CreateCompiler(&compiler));
  IFR(compiler->Disassemble(source, &disassemblyText));
  IFR(disassemblyText.QueryInterface(ppDisassembly));

  return S_OK;
}

HRESULT WINAPI BridgeD3DReflect(LPCVOID pSrcData, SIZE_T SrcDataSize,
                                REFIID pInterface, void **ppReflector) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcContainerReflection> reflection;
  UINT shaderIdx;

  *ppReflector = nullptr;

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingOnHeapCopy(pSrcData, SrcDataSize, CP_ACP,
                                                &source));
  IFR(CreateContainerReflection(&reflection));
  IFR(reflection->Load(source));
  IFR(reflection->FindFirstPartKind(hlsl::DFCC_DXIL, &shaderIdx));
  IFR(reflection->GetPartReflection(shaderIdx, pInterface,
                                    (void **)ppReflector));
  return S_OK;
}

HRESULT WINAPI BridgeReadFileToBlob(LPCWSTR pFileName, ID3DBlob **ppContents) {
  if (!ppContents)
    return E_INVALIDARG;
  *ppContents = nullptr;

  CComPtr<IDxcLibrary> library;
  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobFromFile(pFileName, CP_ACP,
                                  (IDxcBlobEncoding **)ppContents));

  return S_OK;
}

HRESULT PreprocessFromBlob(IDxcBlobEncoding *pSource, LPCWSTR pSourceName,
                           const D3D_SHADER_MACRO *pDefines,
                           IDxcIncludeHandler *pInclude, ID3DBlob **ppCodeText,
                           ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcCompiler> compiler;
  CComPtr<IDxcOperationResult> operationResult;
  HRESULT hr;

  try {
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

    IFR(CreateCompiler(&compiler));
    IFR(compiler->Preprocess(pSource, pSourceName, arguments.data(),
                             (UINT)arguments.size(), defines.data(),
                             (UINT)defines.size(), pInclude, &operationResult));
  } catch (const std::bad_alloc &) {
    return E_OUTOFMEMORY;
  } catch (const CAtlException &err) {
    return err.m_hr;
  }

  operationResult->GetStatus(&hr);
  if (SUCCEEDED(hr)) {
    return operationResult->GetResult((IDxcBlob **)ppCodeText);
  } else {
    if (ppErrorMsgs)
      operationResult->GetErrorBuffer((IDxcBlobEncoding **)ppErrorMsgs);
    return hr;
  }
}

HRESULT WINAPI BridgeD3DPreprocess(LPCVOID pSrcData, SIZE_T SrcDataSize,
                                   LPCSTR pSourceName,
                                   const D3D_SHADER_MACRO *pDefines,
                                   ID3DInclude *pInclude, ID3DBlob **ppCodeText,
                                   ID3DBlob **ppErrorMsgs) {
  CComPtr<IDxcLibrary> library;
  CComPtr<IDxcBlobEncoding> source;
  CComPtr<IDxcIncludeHandler> includeHandler;

  *ppCodeText = nullptr;
  if (ppErrorMsgs != nullptr)
    *ppErrorMsgs = nullptr;

  IFR(CreateLibrary(&library));
  IFR(library->CreateBlobWithEncodingFromPinned(pSrcData, SrcDataSize, CP_ACP,
                                                &source));

  // Until we actually wrap the include handler, fail if there's a user-supplied
  // handler.
  if (D3D_COMPILE_STANDARD_FILE_INCLUDE == pInclude) {
    IFR(library->CreateIncludeHandler(&includeHandler));
  } else if (pInclude) {
    return E_INVALIDARG;
  }

  try {
    CA2W pFileName(pSourceName);
    return PreprocessFromBlob(source, pFileName, pDefines, includeHandler,
                              ppCodeText, ppErrorMsgs);
  } catch (const std::bad_alloc &) {
    return E_OUTOFMEMORY;
  } catch (const CAtlException &err) {
    return err.m_hr;
  }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD Reason, LPVOID) { return TRUE; }
