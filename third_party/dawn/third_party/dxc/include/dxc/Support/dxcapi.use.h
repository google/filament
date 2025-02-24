//////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.use.h                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides support for DXC API users.                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXCAPI_USE_H__
#define __DXCAPI_USE_H__

#include "dxc/dxcapi.h"

namespace dxc {

extern const char *kDxCompilerLib;
extern const char *kDxilLib;

// Helper class to dynamically load the dxcompiler or a compatible libraries.
class DxcDllSupport {
protected:
  HMODULE m_dll;
  DxcCreateInstanceProc m_createFn;
  DxcCreateInstance2Proc m_createFn2;

  HRESULT InitializeInternal(LPCSTR dllName, LPCSTR fnName) {
    if (m_dll != nullptr)
      return S_OK;

#ifdef _WIN32
    m_dll = LoadLibraryA(dllName);
    if (m_dll == nullptr)
      return HRESULT_FROM_WIN32(GetLastError());
    m_createFn = (DxcCreateInstanceProc)GetProcAddress(m_dll, fnName);

    if (m_createFn == nullptr) {
      HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
      FreeLibrary(m_dll);
      m_dll = nullptr;
      return hr;
    }
#else
    m_dll = ::dlopen(dllName, RTLD_LAZY);
    if (m_dll == nullptr)
      return E_FAIL;
    m_createFn = (DxcCreateInstanceProc)::dlsym(m_dll, fnName);

    if (m_createFn == nullptr) {
      ::dlclose(m_dll);
      m_dll = nullptr;
      return E_FAIL;
    }
#endif

    // Only basic functions used to avoid requiring additional headers.
    m_createFn2 = nullptr;
    char fnName2[128];
    size_t s = strlen(fnName);
    if (s < sizeof(fnName2) - 2) {
      memcpy(fnName2, fnName, s);
      fnName2[s] = '2';
      fnName2[s + 1] = '\0';
#ifdef _WIN32
      m_createFn2 = (DxcCreateInstance2Proc)GetProcAddress(m_dll, fnName2);
#else
      m_createFn2 = (DxcCreateInstance2Proc)::dlsym(m_dll, fnName2);
#endif
    }

    return S_OK;
  }

public:
  DxcDllSupport() : m_dll(nullptr), m_createFn(nullptr), m_createFn2(nullptr) {}

  DxcDllSupport(DxcDllSupport &&other) {
    m_dll = other.m_dll;
    other.m_dll = nullptr;
    m_createFn = other.m_createFn;
    other.m_createFn = nullptr;
    m_createFn2 = other.m_createFn2;
    other.m_createFn2 = nullptr;
  }

  ~DxcDllSupport() { Cleanup(); }

  HRESULT Initialize() {
    return InitializeInternal(kDxCompilerLib, "DxcCreateInstance");
  }

  HRESULT InitializeForDll(LPCSTR dll, LPCSTR entryPoint) {
    return InitializeInternal(dll, entryPoint);
  }

  template <typename TInterface>
  HRESULT CreateInstance(REFCLSID clsid, TInterface **pResult) {
    return CreateInstance(clsid, __uuidof(TInterface), (IUnknown **)pResult);
  }

  HRESULT CreateInstance(REFCLSID clsid, REFIID riid, IUnknown **pResult) {
    if (pResult == nullptr)
      return E_POINTER;
    if (m_dll == nullptr)
      return E_FAIL;
    HRESULT hr = m_createFn(clsid, riid, (LPVOID *)pResult);
    return hr;
  }

  template <typename TInterface>
  HRESULT CreateInstance2(IMalloc *pMalloc, REFCLSID clsid,
                          TInterface **pResult) {
    return CreateInstance2(pMalloc, clsid, __uuidof(TInterface),
                           (IUnknown **)pResult);
  }

  HRESULT CreateInstance2(IMalloc *pMalloc, REFCLSID clsid, REFIID riid,
                          IUnknown **pResult) {
    if (pResult == nullptr)
      return E_POINTER;
    if (m_dll == nullptr)
      return E_FAIL;
    if (m_createFn2 == nullptr)
      return E_FAIL;
    HRESULT hr = m_createFn2(pMalloc, clsid, riid, (LPVOID *)pResult);
    return hr;
  }

  bool HasCreateWithMalloc() const { return m_createFn2 != nullptr; }

  bool IsEnabled() const { return m_dll != nullptr; }

  void Cleanup() {
    if (m_dll != nullptr) {
      m_createFn = nullptr;
      m_createFn2 = nullptr;
#ifdef _WIN32
      FreeLibrary(m_dll);
#else
      ::dlclose(m_dll);
#endif
      m_dll = nullptr;
    }
  }

  HMODULE Detach() {
    HMODULE hModule = m_dll;
    m_dll = nullptr;
    return hModule;
  }
};

inline DxcDefine GetDefine(LPCWSTR name, LPCWSTR value) {
  DxcDefine result;
  result.Name = name;
  result.Value = value;
  return result;
}

// Checks an HRESULT and formats an error message with the appended data.
void IFT_Data(HRESULT hr, LPCWSTR data);

void EnsureEnabled(DxcDllSupport &dxcSupport);
void ReadFileIntoBlob(DxcDllSupport &dxcSupport, LPCWSTR pFileName,
                      IDxcBlobEncoding **ppBlobEncoding);
void WriteBlobToConsole(IDxcBlob *pBlob, DWORD streamType = STD_OUTPUT_HANDLE);
void WriteBlobToFile(IDxcBlob *pBlob, LPCWSTR pFileName, UINT32 textCodePage);
void WriteBlobToHandle(IDxcBlob *pBlob, HANDLE hFile, LPCWSTR pFileName,
                       UINT32 textCodePage);
void WriteUtf8ToConsole(const char *pText, int charCount,
                        DWORD streamType = STD_OUTPUT_HANDLE);
void WriteUtf8ToConsoleSizeT(const char *pText, size_t charCount,
                             DWORD streamType = STD_OUTPUT_HANDLE);
void WriteOperationErrorsToConsole(IDxcOperationResult *pResult,
                                   bool outputWarnings);
void WriteOperationResultToConsole(IDxcOperationResult *pRewriteResult,
                                   bool outputWarnings);

} // namespace dxc

#endif
