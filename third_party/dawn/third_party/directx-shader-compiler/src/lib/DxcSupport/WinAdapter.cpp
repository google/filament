//===-- WinAdapter.cpp - Windows Adapter for other platforms ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "assert.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#ifndef _WIN32

#include "dxc/Support/Unicode.h"

//===--------------------------- CAllocator -------------------------------===//

void *CAllocator::Reallocate(void *p, size_t nBytes) throw() {
  return realloc(p, nBytes);
}
void *CAllocator::Allocate(size_t nBytes) throw() { return malloc(nBytes); }
void CAllocator::Free(void *p) throw() { free(p); }

//===--------------------------- BSTR Allocation --------------------------===//

void SysFreeString(BSTR bstrString) {
  if (bstrString)
    free((void *)((uintptr_t)bstrString - sizeof(uint32_t)));
}

// Allocate string with length prefix
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/automat/bstr
BSTR SysAllocStringLen(const OLECHAR *strIn, UINT ui) {
  uint32_t *blobOut =
      (uint32_t *)malloc(sizeof(uint32_t) + (ui + 1) * sizeof(OLECHAR));

  if (!blobOut)
    return nullptr;

  // Size in bytes without trailing NULL character
  blobOut[0] = ui * sizeof(OLECHAR);

  BSTR strOut = (BSTR)&blobOut[1];

  if (strIn)
    memcpy(strOut, strIn, blobOut[0]);

  // Write trailing NULL character:
  strOut[ui] = 0;

  return strOut;
}
//===--------------------------- BSTR Length ------------------------------===//
unsigned int SysStringLen(const BSTR bstrString) {
  if (!bstrString)
    return 0;

  uint32_t *blobIn = (uint32_t *)((uintptr_t)bstrString - sizeof(uint32_t));

  return blobIn[0] / sizeof(OLECHAR);
}

//===--------------------------- CHandle -------------------------------===//

CHandle::CHandle(HANDLE h) { m_h = h; }
CHandle::~CHandle() { CloseHandle(m_h); }
CHandle::operator HANDLE() const throw() { return m_h; }

// CComBSTR
CComBSTR::CComBSTR(int nSize, LPCWSTR sz) {
  if (nSize < 0) {
    throw std::invalid_argument("CComBSTR must have size >= 0");
  }

  if (nSize == 0) {
    m_str = NULL;
  } else {
    m_str = SysAllocStringLen(sz, nSize);
    if (!*this) {
      std::runtime_error("out of memory");
    }
  }
}

bool CComBSTR::operator==(const CComBSTR &bstrSrc) const throw() {
  return wcscmp(m_str, bstrSrc.m_str) == 0;
}

//===--------------------------- WArgV -------------------------------===//
WArgV::WArgV(int argc, const char **argv)
    : WStringVector(argc), WCharPtrVector(argc) {
  for (int i = 0; i < argc; ++i) {
    std::string S(argv[i]);
    const int wideLength = ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, S.data(), S.size(), nullptr, 0);
    assert(wideLength > 0 &&
           "else it should have failed during size calculation");
    WStringVector[i].resize(wideLength);
    ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, S.data(), S.size(),
                          &(WStringVector[i])[0], WStringVector[i].size());
    WCharPtrVector[i] = WStringVector[i].data();
  }
}

#endif
