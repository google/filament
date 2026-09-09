///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// FileIOHelper.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utitlity functions to work with files.                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Global.h"
#include "dxc/Support/WinIncludes.h"

#ifndef _ATL_DECLSPEC_ALLOCATOR
#define _ATL_DECLSPEC_ALLOCATOR
#endif

// Forward declarations.
struct IDxcBlob;
struct IDxcBlobEncoding;
struct IDxcBlobUtf8;
struct IDxcBlobWide;

namespace hlsl {

IMalloc *GetGlobalHeapMalloc() throw();

class CDxcThreadMallocAllocator {
public:
  _ATL_DECLSPEC_ALLOCATOR
  static void *Reallocate(void *p, size_t nBytes) throw() {
    return DxcGetThreadMallocNoRef()->Realloc(p, nBytes);
  }

  _ATL_DECLSPEC_ALLOCATOR
  static void *Allocate(size_t nBytes) throw() {
    return DxcGetThreadMallocNoRef()->Alloc(nBytes);
  }

  static void Free(void *p) throw() {
    return DxcGetThreadMallocNoRef()->Free(p);
  }
};

// Like CComHeapPtr, but with CDxcThreadMallocAllocator.
template <typename T>
class CDxcTMHeapPtr : public CHeapPtr<T, CDxcThreadMallocAllocator> {
public:
  CDxcTMHeapPtr() throw() {}

  explicit CDxcTMHeapPtr(T *pData) throw()
      : CHeapPtr<T, CDxcThreadMallocAllocator>(pData) {}
};

// Like CComHeapPtr, but with a stateful allocator.
template <typename T> class CDxcMallocHeapPtr {
private:
  CComPtr<IMalloc> m_pMalloc;

public:
  T *m_pData;

  CDxcMallocHeapPtr(IMalloc *pMalloc) throw()
      : m_pMalloc(pMalloc), m_pData(nullptr) {}

  ~CDxcMallocHeapPtr() {
    if (m_pData)
      m_pMalloc->Free(m_pData);
  }

  operator T *() const throw() { return m_pData; }

  IMalloc *GetMallocNoRef() const throw() { return m_pMalloc.p; }

  bool Allocate(SIZE_T ElementCount) throw() {
    ATLASSERT(m_pData == NULL);
    SIZE_T nBytes = ElementCount * sizeof(T);
    m_pData = static_cast<T *>(m_pMalloc->Alloc(nBytes));
    if (m_pData == NULL)
      return false;
    return true;
  }

  void AllocateBytes(SIZE_T ByteCount) throw() {
    if (m_pData)
      m_pMalloc->Free(m_pData);
    m_pData = static_cast<T *>(m_pMalloc->Alloc(ByteCount));
  }

  // Attach to an existing pointer (takes ownership)
  void Attach(T *pData) throw() {
    m_pMalloc->Free(m_pData);
    m_pData = pData;
  }

  // Detach the pointer (releases ownership)
  T *Detach() throw() {
    T *pTemp = m_pData;
    m_pData = NULL;
    return pTemp;
  }

  // Free the memory pointed to, and set the pointer to NULL
  void Free() throw() {
    m_pMalloc->Free(m_pData);
    m_pData = NULL;
  }
};

HRESULT ReadBinaryFile(IMalloc *pMalloc, LPCWSTR pFileName, void **ppData,
                       DWORD *pDataSize) throw();
HRESULT ReadBinaryFile(LPCWSTR pFileName, void **ppData,
                       DWORD *pDataSize) throw();
HRESULT WriteBinaryFile(LPCWSTR pFileName, const void *pData,
                        DWORD DataSize) throw();

///////////////////////////////////////////////////////////////////////////////
// Blob and encoding manipulation functions.

UINT32 DxcCodePageFromBytes(const char *bytes, size_t byteLen) throw();

// More general create blob functions, used by other functions
// Null pMalloc means use current thread malloc.
// bPinned will point to existing memory without managing it;
// bCopy will copy to heap; bPinned and bCopy are mutually exclusive.
// If encodingKnown, UTF-8 or wide, and null-termination possible,
// an IDxcBlobUtf8 or IDxcBlobWide will be constructed.
// If text, it's best if size includes null terminator when not copying,
// otherwise IDxcBlobUtf8 or IDxcBlobWide will not be constructed.
HRESULT DxcCreateBlob(LPCVOID pPtr, SIZE_T size, bool bPinned, bool bCopy,
                      bool encodingKnown, UINT32 codePage, IMalloc *pMalloc,
                      IDxcBlobEncoding **ppBlobEncoding) throw();
// Create from blob references original blob.
// Pass nonzero for offset or length for sub-blob reference.
HRESULT
DxcCreateBlobEncodingFromBlob(IDxcBlob *pFromBlob, UINT32 offset, UINT32 length,
                              bool encodingKnown, UINT32 codePage,
                              IMalloc *pMalloc,
                              IDxcBlobEncoding **ppBlobEncoding) throw();

// Load files
HRESULT DxcCreateBlobFromFile(IMalloc *pMalloc, LPCWSTR pFileName,
                              UINT32 *pCodePage,
                              IDxcBlobEncoding **pBlobEncoding) throw();

HRESULT DxcCreateBlobFromFile(LPCWSTR pFileName, UINT32 *pCodePage,
                              IDxcBlobEncoding **ppBlobEncoding) throw();

// Given a blob, creates a subrange view.
HRESULT DxcCreateBlobFromBlob(IDxcBlob *pBlob, UINT32 offset, UINT32 length,
                              IDxcBlob **ppResult) throw();

// Creates a blob wrapping a buffer to be freed with the provided IMalloc
HRESULT
DxcCreateBlobOnMalloc(LPCVOID pData, IMalloc *pIMalloc, UINT32 size,
                      IDxcBlob **ppResult) throw();

// Creates a blob with a copy of the provided data
HRESULT
DxcCreateBlobOnHeapCopy(LPCVOID pData, UINT32 size,
                        IDxcBlob **ppResult) throw();

// Given a blob, creates a new instance with a specific code page set.
HRESULT
DxcCreateBlobWithEncodingSet(IDxcBlob *pBlob, UINT32 codePage,
                             IDxcBlobEncoding **ppBlobEncoding) throw();
HRESULT
DxcCreateBlobWithEncodingSet(IMalloc *pMalloc, IDxcBlob *pBlob, UINT32 codePage,
                             IDxcBlobEncoding **ppBlobEncoding) throw();

// Creates a blob around encoded text without ownership transfer
HRESULT
DxcCreateBlobWithEncodingFromPinned(LPCVOID pText, UINT32 size, UINT32 codePage,
                                    IDxcBlobEncoding **pBlobEncoding) throw();

HRESULT DxcCreateBlobFromPinned(LPCVOID pText, UINT32 size,
                                IDxcBlob **pBlob) throw();

HRESULT
DxcCreateBlobWithEncodingFromStream(IStream *pStream, bool newInstanceAlways,
                                    UINT32 codePage,
                                    IDxcBlobEncoding **pBlobEncoding) throw();

// Creates a blob with a copy of the encoded text
HRESULT
DxcCreateBlobWithEncodingOnHeapCopy(LPCVOID pText, UINT32 size, UINT32 codePage,
                                    IDxcBlobEncoding **pBlobEncoding) throw();

// Creates a blob wrapping encoded text to be freed with the provided IMalloc
HRESULT
DxcCreateBlobWithEncodingOnMalloc(LPCVOID pText, IMalloc *pIMalloc, UINT32 size,
                                  UINT32 codePage,
                                  IDxcBlobEncoding **pBlobEncoding) throw();

// Creates a blob with a copy of encoded text, allocated using the provided
// IMalloc
HRESULT
DxcCreateBlobWithEncodingOnMallocCopy(IMalloc *pIMalloc, LPCVOID pText,
                                      UINT32 size, UINT32 codePage,
                                      IDxcBlobEncoding **pBlobEncoding) throw();

HRESULT DxcGetBlobAsUtf8(IDxcBlob *pBlob, IMalloc *pMalloc,
                         IDxcBlobUtf8 **pBlobEncoding,
                         UINT32 defaultCodePage = CP_ACP) throw();
HRESULT
DxcGetBlobAsWide(IDxcBlob *pBlob, IMalloc *pMalloc,
                 IDxcBlobWide **pBlobEncoding) throw();

bool IsBlobNullOrEmpty(IDxcBlob *pBlob) throw();

///////////////////////////////////////////////////////////////////////////////
// Stream implementations.
class AbstractMemoryStream : public IStream {
public:
  virtual LPBYTE GetPtr() throw() = 0;
  virtual ULONG GetPtrSize() throw() = 0;
  virtual LPBYTE Detach() throw() = 0;
  virtual UINT64 GetPosition() throw() = 0;
  virtual HRESULT Reserve(ULONG targetSize) throw() = 0;
};
HRESULT CreateMemoryStream(IMalloc *pMalloc,
                           AbstractMemoryStream **ppResult) throw();
HRESULT CreateReadOnlyBlobStream(IDxcBlob *pSource, IStream **ppResult) throw();
HRESULT CreateFixedSizeMemoryStream(LPBYTE pBuffer, size_t size,
                                    AbstractMemoryStream **ppResult) throw();

template <typename T>
HRESULT WriteStreamValue(IStream *pStream, const T &value) {
  ULONG cb;
  return pStream->Write(&value, sizeof(value), &cb);
}

} // namespace hlsl
