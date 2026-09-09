///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxlib_sample.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements compile function which compile shader to lib then link.        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/WinIncludes.h"
#include "lib_share_helper.h"

using namespace hlsl;

// Overwrite new delete copy from DXCompiler.cpp
// C++ exception specification ignored except to indicate a function is not
// __declspec(nothrow)
#pragma warning(disable : 4290)

// operator new and friends.
void *__CRTDECL operator new(std::size_t size) noexcept(false) {
  void *ptr = DxcNew(size);
  if (ptr == nullptr)
    throw std::bad_alloc();
  return ptr;
}
void *__CRTDECL operator new(std::size_t size,
                             const std::nothrow_t &nothrow_value) throw() {
  return DxcNew(size);
}
void __CRTDECL operator delete(void *ptr) throw() { DxcDelete(ptr); }
void __CRTDECL operator delete(void *ptr,
                               const std::nothrow_t &nothrow_constant) throw() {
  DxcDelete(ptr);
}
// Finish of new delete.

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD Reason, LPVOID) {
  BOOL result = TRUE;
  if (Reason == DLL_PROCESS_ATTACH) {
    DxcInitThreadMalloc();
    DxcSetThreadMallocToDefault();

    if (hlsl::options::initHlslOptTable()) {
      DxcClearThreadMalloc();
      return FALSE;
    } else {
      DxcClearThreadMalloc();
      return TRUE;
    }
  } else if (Reason == DLL_PROCESS_DETACH) {
    DxcSetThreadMallocToDefault();
    libshare::LibCacheManager::ReleaseLibCacheManager();
    ::hlsl::options::cleanupHlslOptTable();
    DxcClearThreadMalloc();
    DxcCleanupThreadMalloc();
  }

  return result;
}
