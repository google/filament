///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DXCompiler.cpp                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the entry point for the dxcompiler DLL.                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/config.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#ifdef LLVM_ON_WIN32
#include "dxcetw.h"
#endif

namespace hlsl {
HRESULT SetupRegistryPassForHLSL();
HRESULT SetupRegistryPassForPIX();
} // namespace hlsl

// C++ exception specification ignored except to indicate a function is not
// __declspec(nothrow)
#pragma warning(disable : 4290)

#if defined(LLVM_ON_WIN32) && !defined(DXC_DISABLE_ALLOCATOR_OVERRIDES)
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
#endif

static HRESULT InitMaybeFail() throw() {
  HRESULT hr;
  bool fsSetup = false, memSetup = false;
  IFC(DxcInitThreadMalloc());
  DxcSetThreadMallocToDefault();
  memSetup = true;
  if (::llvm::sys::fs::SetupPerThreadFileSystem()) {
    hr = E_FAIL;
    goto Cleanup;
  }
  fsSetup = true;
  IFC(hlsl::SetupRegistryPassForHLSL());
  IFC(hlsl::SetupRegistryPassForPIX());
  if (hlsl::options::initHlslOptTable()) {
    hr = E_FAIL;
    goto Cleanup;
  }
Cleanup:
  if (FAILED(hr)) {
    if (fsSetup) {
      ::llvm::sys::fs::CleanupPerThreadFileSystem();
    }
    if (memSetup) {
      DxcClearThreadMalloc();
      DxcCleanupThreadMalloc();
    }
  } else {
    DxcClearThreadMalloc();
  }
  return hr;
}
#if defined(LLVM_ON_UNIX)
HRESULT __attribute__((constructor)) DllMain() { return InitMaybeFail(); }

void __attribute__((destructor)) DllShutdown() {
  DxcSetThreadMallocToDefault();
  ::hlsl::options::cleanupHlslOptTable();
  ::llvm::sys::fs::CleanupPerThreadFileSystem();
  ::llvm::llvm_shutdown();
  DxcClearThreadMalloc();
  DxcCleanupThreadMalloc();
}
#else  // LLVM_ON_UNIX
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD Reason, LPVOID reserved) {
  BOOL result = TRUE;
  if (Reason == DLL_PROCESS_ATTACH) {
    EventRegisterMicrosoft_Windows_DXCompiler_API();
    DxcEtw_DXCompilerInitialization_Start();
    HRESULT hr = InitMaybeFail();
    DxcEtw_DXCompilerInitialization_Stop(hr);
    result = SUCCEEDED(hr) ? TRUE : FALSE;
  } else if (Reason == DLL_PROCESS_DETACH) {
    DxcEtw_DXCompilerShutdown_Start();
    DxcSetThreadMallocToDefault();
    ::hlsl::options::cleanupHlslOptTable();
    ::llvm::sys::fs::CleanupPerThreadFileSystem();
    ::llvm::llvm_shutdown();
    DxcClearThreadMalloc();
    DxcCleanupThreadMalloc();
    DxcEtw_DXCompilerShutdown_Stop(S_OK);
    EventUnregisterMicrosoft_Windows_DXCompiler_API();
  }

  return result;
}
#endif // LLVM_ON_UNIX
