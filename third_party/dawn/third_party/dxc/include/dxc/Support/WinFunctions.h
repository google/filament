//===-- WinFunctions.h - Windows Functions for other platforms --*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_WINFUNCTIONS_H
#define LLVM_SUPPORT_WINFUNCTIONS_H

#include "dxc/WinAdapter.h"

#ifndef _WIN32

HRESULT StringCchPrintfA(char *dst, size_t dstSize, const char *format, ...);
HRESULT UIntAdd(UINT uAugend, UINT uAddend, UINT *puResult);
HRESULT IntToUInt(int in, UINT *out);
HRESULT SizeTToInt(size_t in, INT *out);
HRESULT UInt32Mult(UINT a, UINT b, UINT *out);

int strnicmp(const char *str1, const char *str2, size_t count);
int _stricmp(const char *str1, const char *str2);
int _wcsicmp(const wchar_t *str1, const wchar_t *str2);
int _wcsnicmp(const wchar_t *str1, const wchar_t *str2, size_t n);
int wsprintf(wchar_t *wcs, const wchar_t *fmt, ...);
unsigned char _BitScanForward(unsigned long *Index, unsigned long Mask);

HANDLE CreateFile2(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   DWORD dwCreationDisposition, void *pCreateExParams);

HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                   void *lpSecurityAttributes, DWORD dwCreationDisposition,
                   DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);

BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
              LPDWORD lpNumberOfBytesRead, void *lpOverlapped);
BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
               LPDWORD lpNumberOfBytesWritten, void *lpOverlapped);

BOOL CloseHandle(HANDLE hObject);

// Windows-specific heap functions
HANDLE HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
BOOL HeapDestroy(HANDLE heap);
LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T nBytes);
LPVOID HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
SIZE_T HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
HANDLE GetProcessHeap();

#endif // _WIN32

#endif // LLVM_SUPPORT_WINFUNCTIONS_H
