//===- WinIncludes.cpp -----------------------------------------*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// WinIncludes.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"

#include "assert.h"
#include "dxc/Support/microcom.h"

#if defined(_WIN32) && !defined(DXC_DISABLE_ALLOCATOR_OVERRIDES)
// CoGetMalloc from combaseapi.h is used
#else
struct DxcCoMalloc : public IMalloc {
  DxcCoMalloc() : m_dwRef(0){};

  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
    assert(false && "QueryInterface not implemented for DxcCoMalloc.");
    return E_NOINTERFACE;
  }

  void *STDMETHODCALLTYPE Alloc(SIZE_T size) override { return malloc(size); }
  void *STDMETHODCALLTYPE Realloc(void *ptr, SIZE_T size) override {
    return realloc(ptr, size);
  }
  void STDMETHODCALLTYPE Free(void *ptr) override { free(ptr); }
  SIZE_T STDMETHODCALLTYPE GetSize(void *pv) override { return -1; }
  int STDMETHODCALLTYPE DidAlloc(void *pv) override { return -1; }
  void STDMETHODCALLTYPE HeapMinimize(void) override {}

private:
  DXC_MICROCOM_REF_FIELD(m_dwRef)
};

HRESULT DxcCoGetMalloc(DWORD dwMemContext, IMalloc **ppMalloc) {
  *ppMalloc = new DxcCoMalloc;
  (*ppMalloc)->AddRef();
  return S_OK;
}
#endif