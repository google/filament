///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxillib.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides access to dxil.dll                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxillib.h"
#include "dxc/Support/Global.h" // For DXASSERT
#include "dxc/Support/dxcapi.use.h"
#include "llvm/Support/Mutex.h"

using namespace dxc;

static DxcDllSupport g_DllSupport;
static HRESULT g_DllLibResult = S_OK;

static llvm::sys::Mutex *cs = nullptr;

// Check if we can successfully get IDxcValidator from dxil.dll
// This function is to prevent multiple attempts to load dxil.dll
HRESULT DxilLibInitialize() {
  cs = new llvm::sys::Mutex;
  cs->lock();
  g_DllLibResult = g_DllSupport.InitializeForDll(kDxilLib, "DxcCreateInstance");
  cs->unlock();
  return S_OK;
}

HRESULT DxilLibCleanup(DxilLibCleanUpType type) {
  HRESULT hr = S_OK;
  if (type == DxilLibCleanUpType::ProcessTermination) {
    g_DllSupport.Detach();
  } else if (type == DxilLibCleanUpType::UnloadLibrary) {
    g_DllSupport.Cleanup();
  } else {
    hr = E_INVALIDARG;
  }
  delete cs;
  cs = nullptr;
  return hr;
}

// g_DllLibResult is S_OK by default, check again to see if dxil.dll is loaded
// If we fail to load dxil.dll, set g_DllLibResult to E_FAIL so that we don't
// have multiple attempts to load dxil.dll
bool DxilLibIsEnabled() {
  cs->lock();
  if (SUCCEEDED(g_DllLibResult)) {
    if (!g_DllSupport.IsEnabled()) {
      g_DllLibResult =
          g_DllSupport.InitializeForDll(kDxilLib, "DxcCreateInstance");
    }
  }
  cs->unlock();
  return SUCCEEDED(g_DllLibResult);
}

HRESULT DxilLibCreateInstance(REFCLSID rclsid, REFIID riid,
                              IUnknown **ppInterface) {
  DXASSERT_NOMSG(ppInterface != nullptr);
  HRESULT hr = E_FAIL;
  if (DxilLibIsEnabled()) {
    cs->lock();
    hr = g_DllSupport.CreateInstance(rclsid, riid, ppInterface);
    cs->unlock();
  }
  return hr;
}
