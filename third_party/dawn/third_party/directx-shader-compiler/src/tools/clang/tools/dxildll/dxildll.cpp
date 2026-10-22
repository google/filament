///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxildll.cpp                                                                //
// Copyright (C) Microsoft. All rights reserved.                             //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DxcCreateInstanceand DllMain functions                     //
// for the dxil.dll module.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define DXC_API_IMPORT
#else
#define DXC_API_IMPORT __attribute__((visibility("default")))
#endif

#include "dxc/DxilContainer/DxcContainerBuilder.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include <algorithm>
#ifdef _WIN32
#include "Tracing/DxcRuntimeEtw.h"
#include "dxc/Tracing/dxcetw.h"
#endif

#include "dxc/dxcisense.h"
#include "dxc/dxctools.h"

HRESULT CreateDxcValidator(REFIID, LPVOID *);

// C++ exception specification ignored except to indicate a function is not
// __declspec(nothrow)
static HRESULT InitMaybeFail() throw() {
  bool MemSetup = false;
  HRESULT HR = (DxcInitThreadMalloc());
  if (!DXC_FAILED(HR)) {
    DxcSetThreadMallocToDefault();
    MemSetup = true;
    if (::llvm::sys::fs::SetupPerThreadFileSystem())
      HR = E_FAIL;
  }
  if (FAILED(HR)) {
    if (MemSetup) {
      DxcClearThreadMalloc();
      DxcCleanupThreadMalloc();
    }
  } else
    DxcClearThreadMalloc();
  return HR;
}

#if defined(LLVM_ON_UNIX)
HRESULT __attribute__((constructor)) DllMain() { return InitMaybeFail(); }

void __attribute__((destructor)) DllShutdown() {
  DxcSetThreadMallocToDefault();
  ::llvm::sys::fs::CleanupPerThreadFileSystem();
  ::llvm::llvm_shutdown();
  DxcClearThreadMalloc();
  DxcCleanupThreadMalloc();
}
#else

#pragma warning(disable : 4290)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD Reason, LPVOID) {
  if (Reason == DLL_PROCESS_ATTACH) {
    EventRegisterMicrosoft_Windows_DxcRuntime_API();
    DxcRuntimeEtw_DxcRuntimeInitialization_Start();
    HRESULT HR = InitMaybeFail();
    DxcRuntimeEtw_DxcRuntimeInitialization_Stop(HR);
    if (FAILED(HR)) {
      EventUnregisterMicrosoft_Windows_DxcRuntime_API();
      return HR;
    }
  } else if (Reason == DLL_PROCESS_DETACH) {
    DxcRuntimeEtw_DxcRuntimeShutdown_Start();
    DxcSetThreadMallocToDefault();
    ::llvm::sys::fs::CleanupPerThreadFileSystem();
    ::llvm::llvm_shutdown();
    DxcClearThreadMalloc();
    DxcCleanupThreadMalloc();
    DxcRuntimeEtw_DxcRuntimeShutdown_Stop(S_OK);
    EventUnregisterMicrosoft_Windows_DxcRuntime_API();
  }

  return TRUE;
}

void *__CRTDECL operator new(std::size_t Size) noexcept(false) {
  void *PTR = DxcNew(Size);
  if (PTR == nullptr)
    throw std::bad_alloc();
  return PTR;
}
void *__CRTDECL operator new(std::size_t Size, const std::nothrow_t &) throw() {
  return DxcNew(Size);
}
void __CRTDECL operator delete(void *PTR) throw() { DxcDelete(PTR); }
void __CRTDECL operator delete(void *PTR,
                               const std::nothrow_t &nothrow_constant) throw() {
  DxcDelete(PTR);
}
#endif

static HRESULT CreateDxcHashingContainerBuilder(REFIID RRID, LPVOID *V) {
  // Call dxil.dll's containerbuilder
  *V = nullptr;
  CComPtr<DxcContainerBuilder> Result(
      DxcContainerBuilder::Alloc(DxcGetThreadMallocNoRef()));
  if (nullptr == Result.p)
    return E_OUTOFMEMORY;

  Result->Init();
  return Result->QueryInterface(RRID, V);
}

static HRESULT ThreadMallocDxcCreateInstance(REFCLSID RCLSID, REFIID RIID,
                                             LPVOID *V) {
  *V = nullptr;
  if (IsEqualCLSID(RCLSID, CLSID_DxcValidator))
    return CreateDxcValidator(RIID, V);
  if (IsEqualCLSID(RCLSID, CLSID_DxcContainerBuilder))
    return CreateDxcHashingContainerBuilder(RIID, V);
  return REGDB_E_CLASSNOTREG;
}

DXC_API_IMPORT HRESULT __stdcall DxcCreateInstance(REFCLSID RCLSID, REFIID RIID,
                                                   LPVOID *V) {
  HRESULT HR = S_OK;
  DxcEtw_DXCompilerCreateInstance_Start();
  DxcThreadMalloc TM(nullptr);
  HR = ThreadMallocDxcCreateInstance(RCLSID, RIID, V);
  DxcEtw_DXCompilerCreateInstance_Stop(HR);
  return HR;
}

DXC_API_IMPORT HRESULT __stdcall DxcCreateInstance2(IMalloc *Malloc,
                                                    REFCLSID RCLSID,
                                                    REFIID RIID, LPVOID *V) {
  if (V == nullptr)
    return E_POINTER;
  HRESULT HR = S_OK;
  DxcEtw_DXCompilerCreateInstance_Start();
  DxcThreadMalloc TM(Malloc);
  HR = ThreadMallocDxcCreateInstance(RCLSID, RIID, V);
  DxcEtw_DXCompilerCreateInstance_Stop(HR);
  return HR;
}
