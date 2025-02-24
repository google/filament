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

#define DXC_API_IMPORT __declspec(dllexport)

#include "dxc/Support/Global.h"
#include "dxc/dxcdxrfallbackcompiler.h"
#include "dxc/dxctools.h"
#include "dxcetw.h"
#include <memory>

HRESULT CreateDxcDxrFallbackCompiler(REFIID riid, LPVOID *ppv);

static HRESULT ThreadMallocDxcCreateInstance(REFCLSID rclsid, REFIID riid,
                                             LPVOID *ppv) {
  HRESULT hr = S_OK;
  *ppv = nullptr;

  if (IsEqualCLSID(rclsid, CLSID_DxcDxrFallbackCompiler)) {
    hr = CreateDxcDxrFallbackCompiler(riid, ppv);
  } else {
    hr = REGDB_E_CLASSNOTREG;
  }
  return hr;
}

DXC_API_IMPORT HRESULT __stdcall DxcCreateDxrFallbackCompiler(REFCLSID rclsid,
                                                              REFIID riid,
                                                              LPVOID *ppv) {
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
