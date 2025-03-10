//===-- WinFunctions.cpp - Windows Functions for other platforms --*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines Windows-specific functions used in the codebase for
// non-Windows platforms.
//
//===----------------------------------------------------------------------===//

#ifndef _WIN32
#include <fcntl.h>
#include <map>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/microcom.h"

HRESULT StringCchPrintfA(char *dst, size_t dstSize, const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_list argscopy;
  va_copy(argscopy, args);
  // C++11 snprintf can return the size of the resulting string if it was to be
  // constructed.
  size_t size =
      vsnprintf(nullptr, 0, format, argscopy) + 1; // Extra space for '\0'
  if (size > dstSize) {
    *dst = '\0';
  } else {
    vsnprintf(dst, size, format, args);
  }
  va_end(argscopy);
  va_end(args);
  return S_OK;
}
HRESULT UIntAdd(UINT uAugend, UINT uAddend, UINT *puResult) {
  HRESULT hr;
  if ((uAugend + uAddend) >= uAugend) {
    *puResult = (uAugend + uAddend);
    hr = S_OK;
  } else {
    *puResult = 0xffffffff;
    hr = ERROR_ARITHMETIC_OVERFLOW;
  }
  return hr;
}
HRESULT IntToUInt(int in, UINT *out) {
  HRESULT hr;
  if (in >= 0) {
    *out = (UINT)in;
    hr = S_OK;
  } else {
    *out = 0xffffffff;
    hr = ERROR_ARITHMETIC_OVERFLOW;
  }
  return hr;
}
HRESULT SizeTToInt(size_t in, int *out) {
  HRESULT hr;
  if (in <= INT_MAX) {
    *out = (int)in;
    hr = S_OK;
  } else {
    *out = 0xffffffff;
    hr = ERROR_ARITHMETIC_OVERFLOW;
  }
  return hr;
}
HRESULT UInt32Mult(UINT a, UINT b, UINT *out) {
  uint64_t result = (uint64_t)a * (uint64_t)b;
  if (result > uint64_t(UINT_MAX))
    return ERROR_ARITHMETIC_OVERFLOW;

  *out = (uint32_t)result;
  return S_OK;
}

int strnicmp(const char *str1, const char *str2, size_t count) {
  size_t i = 0;
  for (; i < count && str1[i] && str2[i]; ++i) {
    int d = std::tolower(str1[i]) - std::tolower(str2[i]);
    if (d != 0)
      return d;
  }

  if (i == count) {
    // All 'count' characters matched.
    return 0;
  }

  // str1 or str2 reached NULL before 'count' characters were compared.
  return str1[i] - str2[i];
}

int _stricmp(const char *str1, const char *str2) {
  size_t i = 0;
  for (; str1[i] && str2[i]; ++i) {
    int d = std::tolower(str1[i]) - std::tolower(str2[i]);
    if (d != 0)
      return d;
  }
  return str1[i] - str2[i];
}

int _wcsicmp(const wchar_t *str1, const wchar_t *str2) {
  size_t i = 0;
  for (; str1[i] && str2[i]; ++i) {
    int d = std::towlower(str1[i]) - std::towlower(str2[i]);
    if (d != 0)
      return d;
  }
  return str1[i] - str2[i];
}

int _wcsnicmp(const wchar_t *str1, const wchar_t *str2, size_t n) {
  size_t i = 0;
  for (; i < n && str1[i] && str2[i]; ++i) {
    int d = std::towlower(str1[i]) - std::towlower(str2[i]);
    if (d != 0)
      return d;
  }
  if (i >= n)
    return 0;
  return str1[i] - str2[i];
}

unsigned char _BitScanForward(unsigned long *Index, unsigned long Mask) {
  unsigned long l;
  if (!Mask)
    return 0;
  for (l = 0; !(Mask & 1); l++)
    Mask >>= 1;
  *Index = l;
  return 1;
}

HANDLE CreateFile2(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   DWORD dwCreationDisposition, void *pCreateExParams) {
  return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, pCreateExParams,
                     dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
}

HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   void *lpSecurityAttributes, DWORD dwCreationDisposition,
                   DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  CW2A pUtf8FileName(lpFileName);
  size_t fd = -1;
  int flags = 0;

  if (dwDesiredAccess & GENERIC_WRITE)
    if (dwDesiredAccess & GENERIC_READ)
      flags |= O_RDWR;
    else
      flags |= O_WRONLY;
  else // dwDesiredAccess may be 0, but open() demands something here. This is
       // mostly harmless
    flags |= O_RDONLY;

  if (dwCreationDisposition == CREATE_ALWAYS)
    flags |= (O_CREAT | O_TRUNC);
  if (dwCreationDisposition == OPEN_ALWAYS)
    flags |= O_CREAT;
  else if (dwCreationDisposition == CREATE_NEW)
    flags |= (O_CREAT | O_EXCL);
  else if (dwCreationDisposition == TRUNCATE_EXISTING)
    flags |= O_TRUNC;
  // OPEN_EXISTING represents default open() behavior

  // Catch Implementation limitations.
  assert(!lpSecurityAttributes &&
         "security attributes not supported in CreateFileW yet");
  assert(!hTemplateFile && "template file not supported in CreateFileW yet");
  assert(dwFlagsAndAttributes == FILE_ATTRIBUTE_NORMAL &&
         "Attributes other than NORMAL not supported in CreateFileW yet");

  while ((int)(fd = open(pUtf8FileName, flags,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    if (errno != EINTR)
      return INVALID_HANDLE_VALUE;
  }

  return (HANDLE)fd;
}

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
  int fd = (size_t)hFile;
  struct stat fdstat;
  int rv = fstat(fd, &fdstat);
  if (!rv) {
    lpFileSize->QuadPart = (LONGLONG)fdstat.st_size;
    return true;
  }
  return false;
}

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
              LPDWORD lpNumberOfBytesRead, void *lpOverlapped) {
  size_t fd = (size_t)hFile;
  ssize_t rv = -1;

  // Implementation limitation
  assert(!lpOverlapped && "Overlapping not supported in ReadFile yet.");

  rv = read(fd, lpBuffer, nNumberOfBytesToRead);
  if (rv < 0)
    return false;
  *lpNumberOfBytesRead = rv;
  return true;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
               LPDWORD lpNumberOfBytesWritten, void *lpOverlapped) {
  size_t fd = (size_t)hFile;
  ssize_t rv = -1;

  // Implementation limitation
  assert(!lpOverlapped && "Overlapping not supported in WriteFile yet.");

  rv = write(fd, lpBuffer, nNumberOfBytesToWrite);
  if (rv < 0)
    return false;
  *lpNumberOfBytesWritten = rv;
  return true;
}

BOOL CloseHandle(HANDLE hObject) {
  int fd = (size_t)hObject;
  return !close(fd);
}

// Half-hearted implementation of a heap structure
// Enables size queries, maximum allocation limit, and collective free at heap
// destruction Does not perform any preallocation or allocation organization.
// Does not respect any flags except for HEAP_ZERO_MEMORY
struct SimpleAllocation {
  LPVOID ptr;
  SIZE_T size;
};

struct SimpleHeap {
  std::map<LPCVOID, SimpleAllocation> allocs;
  SIZE_T maxSize, curSize;
};

HANDLE HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) {
  SimpleHeap *simpHeap = new SimpleHeap;
  simpHeap->maxSize = dwMaximumSize;
  simpHeap->curSize = 0;
  return (HANDLE)simpHeap;
}

BOOL HeapDestroy(HANDLE hHeap) {
  SimpleHeap *simpHeap = (SimpleHeap *)hHeap;

  for (auto it = simpHeap->allocs.begin(), e = simpHeap->allocs.end(); it != e;
       it++)
    free(it->second.ptr);

  delete simpHeap;
  return true;
}

LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
  LPVOID ptr = nullptr;
  SimpleHeap *simpHeap = (SimpleHeap *)hHeap;

  if (simpHeap->maxSize && simpHeap->curSize + dwBytes > simpHeap->maxSize)
    return nullptr;

  if (dwFlags == HEAP_ZERO_MEMORY)
    ptr = calloc(1, dwBytes);
  else
    ptr = malloc(dwBytes);

  simpHeap->allocs[ptr] = {ptr, dwBytes};
  simpHeap->curSize += dwBytes;

  return ptr;
}

LPVOID HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes) {
  LPVOID ptr = nullptr;
  SimpleHeap *simpHeap = (SimpleHeap *)hHeap;
  SIZE_T oSize = simpHeap->allocs[lpMem].size;

  if (simpHeap->maxSize &&
      simpHeap->curSize - oSize + dwBytes > simpHeap->maxSize)
    return nullptr;

  ptr = realloc(lpMem, dwBytes);
  if (dwFlags == HEAP_ZERO_MEMORY && oSize < dwBytes)
    memset((char *)ptr + oSize, 0, dwBytes - oSize);

  simpHeap->allocs.erase(lpMem);
  simpHeap->curSize -= oSize;

  simpHeap->allocs[ptr] = {ptr, dwBytes};
  simpHeap->curSize += dwBytes;

  return ptr;
}

BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
  SimpleHeap *simpHeap = (SimpleHeap *)hHeap;
  SIZE_T oSize = simpHeap->allocs[lpMem].size;

  free(lpMem);

  simpHeap->allocs.erase(lpMem);
  simpHeap->curSize -= oSize;

  return true;
}

SIZE_T HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
  SimpleHeap *simpHeap = (SimpleHeap *)hHeap;
  return simpHeap->allocs[lpMem].size;
}

static SimpleHeap g_processHeap;

HANDLE GetProcessHeap() { return (HANDLE)&g_processHeap; }

#endif // _WIN32
