///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxillib.h                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides wrappers to handle calls to dxil.dll                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __DXC_DXILLIB__
#define __DXC_DXILLIB__

#include "dxc/Support/WinIncludes.h"

// Initialize Dxil library.
HRESULT DxilLibInitialize();

// When dxcompiler is detached from process,
// we should not call FreeLibrary on process termination.
// So the caller has to specify if cleaning is from FreeLibrary or process
// termination
enum class DxilLibCleanUpType { UnloadLibrary, ProcessTermination };

HRESULT DxilLibCleanup(DxilLibCleanUpType type);

// Check if can access dxil.dll
bool DxilLibIsEnabled();

HRESULT DxilLibCreateInstance(REFCLSID rclsid, REFIID riid,
                              IUnknown **ppInterface);

template <class TInterface>
HRESULT DxilLibCreateInstance(REFCLSID rclsid, TInterface **ppInterface) {
  return DxilLibCreateInstance(rclsid, __uuidof(TInterface),
                               (IUnknown **)ppInterface);
}

#endif // __DXC_DXILLIB__
