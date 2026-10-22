///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.cpp                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DxcCreateInstance function for the DirectX Compiler.       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"

#ifdef _WIN32
#define DXC_API_IMPORT __declspec(dllexport)
#else
#define DXC_API_IMPORT __attribute__((visibility("default")))
#endif

#include "dxc/Support/Global.h"
#include "dxc/config.h"
#include "dxc/dxcisense.h"
#include "dxc/dxctools.h"
#ifdef _WIN32
#include "dxcetw.h"
#endif
#include "dxc/DxilContainer/DxcContainerBuilder.h"
#include <memory>

HRESULT CreateDxcCompiler(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcDiaDataSource(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcIntelliSense(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcCompilerArgs(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcUtils(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcRewriter(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcValidator(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcAssembler(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcOptimizer(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcContainerBuilder(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcLinker(REFIID riid, _Out_ LPVOID *ppv);
HRESULT CreateDxcPdbUtils(REFIID riid, _Out_ LPVOID *ppv);

namespace hlsl {
void CreateDxcContainerReflection(IDxcContainerReflection **ppResult);
void CreateDxcLinker(IDxcContainerReflection **ppResult);
} // namespace hlsl

HRESULT CreateDxcContainerReflection(REFIID riid, _Out_ LPVOID *ppv) {
  try {
    CComPtr<IDxcContainerReflection> pReflection;
    hlsl::CreateDxcContainerReflection(&pReflection);
    return pReflection->QueryInterface(riid, ppv);
  } catch (const std::bad_alloc &) {
    return E_OUTOFMEMORY;
  }
}

HRESULT CreateDxcContainerBuilder(REFIID riid, _Out_ LPVOID *ppv) {
  // Call dxil.dll's containerbuilder
  *ppv = nullptr;

  CComPtr<DxcContainerBuilder> Result =
      DxcContainerBuilder::Alloc(DxcGetThreadMallocNoRef());
  IFROOM(Result.p);
  Result->Init();
  return Result->QueryInterface(riid, ppv);
}

static HRESULT ThreadMallocDxcCreateInstance(REFCLSID rclsid, REFIID riid,
                                             _Out_ LPVOID *ppv) {
  HRESULT hr = S_OK;
  *ppv = nullptr;
  if (IsEqualCLSID(rclsid, CLSID_DxcCompiler)) {
    hr = CreateDxcCompiler(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcCompilerArgs)) {
    hr = CreateDxcCompilerArgs(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcUtils)) {
    hr = CreateDxcUtils(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcValidator)) {
    hr = CreateDxcValidator(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcAssembler)) {
    hr = CreateDxcAssembler(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcOptimizer)) {
    hr = CreateDxcOptimizer(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcIntelliSense)) {
    hr = CreateDxcIntelliSense(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcContainerBuilder)) {
    hr = CreateDxcContainerBuilder(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcContainerReflection)) {
    hr = CreateDxcContainerReflection(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcPdbUtils)) {
    hr = CreateDxcPdbUtils(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcRewriter)) {
    hr = CreateDxcRewriter(riid, ppv);
  } else if (IsEqualCLSID(rclsid, CLSID_DxcLinker)) {
    hr = CreateDxcLinker(riid, ppv);
  }
// Note: The following targets are not yet enabled for non-Windows platforms.
#ifdef _WIN32
  else if (IsEqualCLSID(rclsid, CLSID_DxcDiaDataSource)) {
    hr = CreateDxcDiaDataSource(riid, ppv);
  }
#endif
  else {
    hr = REGDB_E_CLASSNOTREG;
  }
  return hr;
}

DXC_API_IMPORT HRESULT __stdcall DxcCreateInstance(REFCLSID rclsid, REFIID riid,
                                                   _Out_ LPVOID *ppv) {
  if (ppv == nullptr) {
    return E_POINTER;
  }

  HRESULT hr = S_OK;
  DxcEtw_DXCompilerCreateInstance_Start();
  DxcThreadMalloc TM(nullptr);
  hr = ThreadMallocDxcCreateInstance(rclsid, riid, ppv);
  DxcEtw_DXCompilerCreateInstance_Stop(hr);
  return hr;
}

DXC_API_IMPORT HRESULT __stdcall DxcCreateInstance2(IMalloc *pMalloc,
                                                    REFCLSID rclsid,
                                                    REFIID riid,
                                                    _Out_ LPVOID *ppv) {
  if (ppv == nullptr) {
    return E_POINTER;
  }
#ifdef DXC_DISABLE_ALLOCATOR_OVERRIDES
  if (pMalloc != DxcGetThreadMallocNoRef()) {
    return E_INVALIDARG;
  }
#endif // DXC_DISABLE_ALLOCATOR_OVERRIDES

  HRESULT hr = S_OK;
  DxcEtw_DXCompilerCreateInstance_Start();
  DxcThreadMalloc TM(pMalloc);
  hr = ThreadMallocDxcCreateInstance(rclsid, riid, ppv);
  DxcEtw_DXCompilerCreateInstance_Stop(hr);
  return hr;
}
