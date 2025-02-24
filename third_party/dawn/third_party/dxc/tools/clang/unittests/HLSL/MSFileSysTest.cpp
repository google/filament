///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MSFileSysTest.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the file system abstraction API.                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
// Includes on Windows are highly order dependent.
#include <stdint.h>
#include "dxc/Support/WinIncludes.h"
#include "dxc/Test/HlslTestUtils.h"

#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/Atomic.h"

#include <d3dcommon.h>
#include "dxc/dxcapi.internal.h"

#include <algorithm>
#include <vector>
#include <memory>
// clang-format on

using namespace llvm;
using namespace llvm::sys;
using namespace llvm::sys::fs;

const GUID DECLSPEC_SELECTANY GUID_NULL = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

#define SIMPLE_IUNKNOWN_IMPL1(_IFACE_)                                         \
private:                                                                       \
  volatile std::atomic<llvm::sys::cas_flag> m_dwRef;                           \
                                                                               \
public:                                                                        \
  ULONG STDMETHODCALLTYPE AddRef() override { return (ULONG)++m_dwRef; }       \
  ULONG STDMETHODCALLTYPE Release() override {                                 \
    ULONG result = (ULONG)--m_dwRef;                                           \
    if (result == 0)                                                           \
      delete this;                                                             \
    return result;                                                             \
  }                                                                            \
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)       \
      override {                                                               \
    if (ppvObject == nullptr)                                                  \
      return E_POINTER;                                                        \
    if (IsEqualIID(iid, __uuidof(IUnknown)) ||                                 \
        IsEqualIID(iid, __uuidof(INoMarshal)) ||                               \
        IsEqualIID(iid, __uuidof(_IFACE_))) {                                  \
      *ppvObject = reinterpret_cast<_IFACE_ *>(this);                          \
      reinterpret_cast<_IFACE_ *>(this)->AddRef();                             \
      return S_OK;                                                             \
    }                                                                          \
    return E_NOINTERFACE;                                                      \
  }

class MSFileSysTest {
public:
  BEGIN_TEST_CLASS(MSFileSysTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(CreationWhenInvokedThenNonNull)

  TEST_METHOD(FindFirstWhenInvokedThenHasFile)
  TEST_METHOD(FindFirstWhenInvokedThenFailsIfNoMatch)
  TEST_METHOD(FindNextWhenLastThenNoMatch)
  TEST_METHOD(FindNextWhenExistsThenMatch)

  TEST_METHOD(OpenWhenNewThenZeroSize)
};

static LPWSTR CoTaskMemDup(LPCWSTR text) {
  if (text == nullptr)
    return nullptr;
  size_t len = wcslen(text) + 1;
  LPWSTR result = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t) * len);
  StringCchCopyW(result, len, text);
  return result;
}

class FixedEnumSTATSTG : public IEnumSTATSTG {
  SIMPLE_IUNKNOWN_IMPL1(IEnumSTATSTG)
private:
  std::vector<STATSTG> m_items;
  unsigned m_index;

public:
  FixedEnumSTATSTG(const STATSTG *items, unsigned itemCount) {
    m_dwRef = 0;
    m_index = 0;
    m_items.reserve(itemCount);
    for (unsigned i = 0; i < itemCount; ++i) {
      m_items.push_back(items[i]);
      m_items[i].pwcsName = CoTaskMemDup(m_items[i].pwcsName);
    }
  }
  virtual ~FixedEnumSTATSTG() {
    for (auto &item : m_items)
      CoTaskMemFree(item.pwcsName);
  }
  HRESULT STDMETHODCALLTYPE Next(ULONG celt, STATSTG *rgelt,
                                 ULONG *pceltFetched) override {
    if (celt != 1 || pceltFetched == nullptr)
      return E_NOTIMPL;
    if (m_index >= m_items.size()) {
      *pceltFetched = 0;
      return S_FALSE;
    }

    *pceltFetched = 1;
    *rgelt = m_items[m_index];
    (*rgelt).pwcsName = CoTaskMemDup((*rgelt).pwcsName);
    ++m_index;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Reset(void) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Clone(IEnumSTATSTG **) override {
    return E_NOTIMPL;
  }
};

class MockDxcSystemAccess : public IDxcSystemAccess {
  SIMPLE_IUNKNOWN_IMPL1(IDxcSystemAccess)

public:
  unsigned findCount;
  MockDxcSystemAccess() : m_dwRef(0), findCount(1) {}
  virtual ~MockDxcSystemAccess() {}
  static HRESULT Create(MockDxcSystemAccess **pResult) {
    *pResult = new (std::nothrow) MockDxcSystemAccess();
    if (*pResult == nullptr)
      return E_OUTOFMEMORY;
    (*pResult)->AddRef();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EnumFiles(LPCWSTR fileName,
                                      IEnumSTATSTG **pResult) override {
    wchar_t hlslName[] = L"filename.hlsl";
    wchar_t fxName[] = L"filename2.fx";
    STATSTG items[] = {{hlslName,
                        STGTY_STREAM,
                        {{0, 0}},
                        {0, 0},
                        {0, 0},
                        {0, 0},
                        0,
                        0,
                        GUID_NULL,
                        0,
                        0},
                       {fxName,
                        STGTY_STREAM,
                        {{0, 0}},
                        {0, 0},
                        {0, 0},
                        {0, 0},
                        0,
                        0,
                        GUID_NULL,
                        0,
                        0}};
    unsigned testCount = (unsigned)std::size(items);
    FixedEnumSTATSTG *resultEnum = new (std::nothrow)
        FixedEnumSTATSTG(items, std::min(testCount, findCount));
    if (resultEnum == nullptr) {
      *pResult = nullptr;
      return E_OUTOFMEMORY;
    }
    resultEnum->AddRef();
    *pResult = resultEnum;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OpenStorage(LPCWSTR lpFileName,
                                        DWORD dwDesiredAccess,
                                        DWORD dwShareMode,
                                        DWORD dwCreationDisposition,
                                        DWORD dwFlagsAndAttributes,
                                        IUnknown **pResult) override {
    *pResult = SHCreateMemStream(nullptr, 0);
    return (*pResult == nullptr) ? E_OUTOFMEMORY : S_OK;
  }
  HRESULT STDMETHODCALLTYPE
  SetStorageTime(IUnknown *storage, const FILETIME *lpCreationTime,
                 const FILETIME *lpLastAccessTime,
                 const FILETIME *lpLastWriteTime) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetFileInformationForStorage(
      IUnknown *storage,
      LPBY_HANDLE_FILE_INFORMATION lpFileInformation) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetFileTypeForStorage(IUnknown *storage,
                                                  DWORD *fileType) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE CreateHardLinkInStorage(
      LPCWSTR lpFileName, LPCWSTR lpExistingFileName) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE MoveStorage(LPCWSTR lpExistingFileName,
                                        LPCWSTR lpNewFileName,
                                        DWORD dwFlags) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE
  GetFileAttributesForStorage(LPCWSTR lpFileName, DWORD *pResult) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE DeleteStorage(LPCWSTR lpFileName) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE
  RemoveDirectoryStorage(LPCWSTR lpFileName) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE
  CreateDirectoryStorage(LPCWSTR lpPathName) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetCurrentDirectoryForStorage(DWORD nBufferLength,
                                                          LPWSTR lpBuffer,
                                                          DWORD *len) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetMainModuleFileNameW(DWORD nBufferLength,
                                                   LPWSTR lpBuffer,
                                                   DWORD *len) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetTempStoragePath(DWORD nBufferLength,
                                               LPWSTR lpBuffer,
                                               DWORD *len) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE SupportsCreateSymbolicLink(BOOL *pResult) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE CreateSymbolicLinkInStorage(
      LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName,
      DWORD dwFlags) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE CreateStorageMapping(IUnknown *hFile,
                                                 DWORD flProtect,
                                                 DWORD dwMaximumSizeHigh,
                                                 DWORD dwMaximumSizeLow,
                                                 IUnknown **pResult) override {
    return E_NOTIMPL;
  }
  HRESULT MapViewOfFile(IUnknown *hFileMappingObject, DWORD dwDesiredAccess,
                        DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                        SIZE_T dwNumberOfBytesToMap,
                        ID3D10Blob **pResult) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE OpenStdStorage(int standardFD,
                                           IUnknown **pResult) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetStreamDisplay(ITextFont **textFont,
                                             unsigned *columnCount) override {
    return E_NOTIMPL;
  }
};

void MSFileSysTest::CreationWhenInvokedThenNonNull() {
  CComPtr<MockDxcSystemAccess> access;
  VERIFY_SUCCEEDED(MockDxcSystemAccess::Create(&access));

  MSFileSystem *fileSystem;
  VERIFY_SUCCEEDED(CreateMSFileSystemForIface(access, &fileSystem));
  VERIFY_IS_NOT_NULL(fileSystem);

  delete fileSystem;
}

void MSFileSysTest::FindFirstWhenInvokedThenHasFile() {
  CComPtr<MockDxcSystemAccess> access;
  MockDxcSystemAccess::Create(&access);

  MSFileSystem *fileSystem;
  CreateMSFileSystemForIface(access, &fileSystem);
  WIN32_FIND_DATAW findData;
  HANDLE h = fileSystem->FindFirstFileW(L"foobar", &findData);
  VERIFY_ARE_EQUAL_WSTR(L"filename.hlsl", findData.cFileName);
  VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, h);
  fileSystem->FindClose(h);
  delete fileSystem;
}

void MSFileSysTest::FindFirstWhenInvokedThenFailsIfNoMatch() {
  CComPtr<MockDxcSystemAccess> access;
  MockDxcSystemAccess::Create(&access);
  access->findCount = 0;

  MSFileSystem *fileSystem;
  CreateMSFileSystemForIface(access, &fileSystem);
  WIN32_FIND_DATAW findData;
  HANDLE h = fileSystem->FindFirstFileW(L"foobar", &findData);
  VERIFY_ARE_EQUAL(ERROR_FILE_NOT_FOUND, (long)GetLastError());
  VERIFY_ARE_EQUAL(INVALID_HANDLE_VALUE, h);
  VERIFY_ARE_EQUAL_WSTR(L"", findData.cFileName);
  delete fileSystem;
}

void MSFileSysTest::FindNextWhenLastThenNoMatch() {
  CComPtr<MockDxcSystemAccess> access;
  MockDxcSystemAccess::Create(&access);

  MSFileSystem *fileSystem;
  CreateMSFileSystemForIface(access, &fileSystem);
  WIN32_FIND_DATAW findData;
  HANDLE h = fileSystem->FindFirstFileW(L"foobar", &findData);
  VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, h);
  BOOL findNext = fileSystem->FindNextFileW(h, &findData);
  VERIFY_IS_FALSE(findNext);
  VERIFY_ARE_EQUAL(ERROR_FILE_NOT_FOUND, (long)GetLastError());
  fileSystem->FindClose(h);
  delete fileSystem;
}

void MSFileSysTest::FindNextWhenExistsThenMatch() {
  CComPtr<MockDxcSystemAccess> access;
  MockDxcSystemAccess::Create(&access);
  access->findCount = 2;

  MSFileSystem *fileSystem;
  CreateMSFileSystemForIface(access, &fileSystem);
  WIN32_FIND_DATAW findData;
  HANDLE h = fileSystem->FindFirstFileW(L"foobar", &findData);
  VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, h);
  BOOL findNext = fileSystem->FindNextFileW(h, &findData);
  VERIFY_IS_TRUE(findNext);
  VERIFY_ARE_EQUAL_WSTR(L"filename2.fx", findData.cFileName);
  VERIFY_IS_FALSE(fileSystem->FindNextFileW(h, &findData));
  fileSystem->FindClose(h);
  delete fileSystem;
}

void MSFileSysTest::OpenWhenNewThenZeroSize() {
  CComPtr<MockDxcSystemAccess> access;
  MockDxcSystemAccess::Create(&access);

  MSFileSystem *fileSystem;
  CreateMSFileSystemForIface(access, &fileSystem);
  HANDLE h = fileSystem->CreateFileW(L"new.hlsl", 0, 0, 0, 0);
  VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, h);
  char buf[4];
  DWORD bytesRead;
  VERIFY_IS_TRUE(fileSystem->ReadFile(h, buf, _countof(buf), &bytesRead));
  VERIFY_ARE_EQUAL(0u, bytesRead);
  fileSystem->CloseHandle(h);
  delete fileSystem;
}
