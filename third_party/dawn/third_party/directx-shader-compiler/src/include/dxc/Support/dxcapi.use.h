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

// Interface for common dll operations
class DllLoader {

protected:
  virtual HRESULT CreateInstanceImpl(REFCLSID clsid, REFIID riid,
                                     IUnknown **pResult) = 0;
  virtual HRESULT CreateInstance2Impl(IMalloc *pMalloc, REFCLSID clsid,
                                      REFIID riid, IUnknown **pResult) = 0;
  virtual ~DllLoader() {}

public:
  DllLoader() = default;
  DllLoader(const DllLoader &) = delete;
  DllLoader(DllLoader &&) = delete;

  template <typename TInterface>
  HRESULT CreateInstance(REFCLSID clsid, TInterface **pResult) {
    return CreateInstanceImpl(clsid, __uuidof(TInterface),
                              (IUnknown **)pResult);
  }
  HRESULT CreateInstance(REFCLSID clsid, REFIID riid, IUnknown **pResult) {
    return CreateInstanceImpl(clsid, riid, (IUnknown **)pResult);
  }

  template <typename TInterface>
  HRESULT CreateInstance2(IMalloc *pMalloc, REFCLSID clsid,
                          TInterface **pResult) {
    return CreateInstance2Impl(pMalloc, clsid, __uuidof(TInterface),
                               (IUnknown **)pResult);
  }
  HRESULT CreateInstance2(IMalloc *pMalloc, REFCLSID clsid, REFIID riid,
                          IUnknown **pResult) {
    return CreateInstance2Impl(pMalloc, clsid, riid, (IUnknown **)pResult);
  }

  virtual bool IsEnabled() const = 0;
};

// Helper class to dynamically load the dxcompiler or a compatible libraries.
class SpecificDllLoader : public DllLoader {

  HMODULE m_dll;
  DxcCreateInstanceProc m_createFn;
  DxcCreateInstance2Proc m_createFn2;

  HRESULT InitializeInternal(LPCSTR dllName, LPCSTR fnName) {
    if (m_dll != nullptr)
      return S_OK;

#ifdef _WIN32
    m_dll = LoadLibraryA(
        dllName); // CodeQL [SM01925] This is by design, intended to be used to
                  // test multiple validators versions.
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
    m_dll = ::dlopen(
        dllName, RTLD_LAZY); // CodeQL [SM01925] This is by design, intended to
                             // be used to test multiple validators versions.
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
  SpecificDllLoader()
      : m_dll(nullptr), m_createFn(nullptr), m_createFn2(nullptr) {}

  ~SpecificDllLoader() override { Cleanup(); }

  HRESULT InitializeForDll(LPCSTR dll, LPCSTR entryPoint) {
    return InitializeInternal(dll, entryPoint);
  }

  // Also bring visibility into the interface definition of this function
  // which takes 2 args
  HRESULT CreateInstanceImpl(REFCLSID clsid, REFIID riid,
                             IUnknown **pResult) override {
    if (pResult == nullptr)
      return E_POINTER;
    if (m_dll == nullptr)
      return E_FAIL;
    HRESULT hr = m_createFn(clsid, riid, (LPVOID *)pResult);
    return hr;
  }

  // Also bring visibility into the interface definition of this function
  // which takes 3 args
  HRESULT CreateInstance2Impl(IMalloc *pMalloc, REFCLSID clsid, REFIID riid,
                              IUnknown **pResult) override {
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

  bool IsEnabled() const override { return m_dll != nullptr; }

  bool GetCreateInstanceProcs(DxcCreateInstanceProc *pCreateFn,
                              DxcCreateInstance2Proc *pCreateFn2) const {
    if (pCreateFn == nullptr || pCreateFn2 == nullptr || m_createFn == nullptr)
      return false;
    *pCreateFn = m_createFn;
    *pCreateFn2 = m_createFn2;
    return true;
  }

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

// This class is for instances where we *only* expect
// to load dxcompiler.dll, and nothing else
class DxCompilerDllLoader : public SpecificDllLoader {
public:
  HRESULT Initialize() {
    return InitializeForDll(kDxCompilerLib, "DxcCreateInstance");
  }
};

// This class is for instances where we want to load a
// subset of any dlls that would give DxcLibrary functionality.
// e.g, load a IDxcLibrary object.
// Includes but isn't limited to dxcompiler.dll and dxil.dll
class DXCLibraryDllLoader : public SpecificDllLoader {
public:
  HRESULT Initialize() {
    return InitializeForDll(kDxCompilerLib, "DxcCreateInstance");
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

void EnsureEnabled(DXCLibraryDllLoader &dxcSupport);
void ReadFileIntoBlob(DllLoader &dxcSupport, LPCWSTR pFileName,
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
