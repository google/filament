# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Win32 functions and constants."""

import ctypes
import ctypes.wintypes

GENERIC_WRITE = 0x40000000
CREATE_ALWAYS = 0x00000002
FILE_ATTRIBUTE_NORMAL = 0x00000080
LOCKFILE_EXCLUSIVE_LOCK = 0x00000002
LOCKFILE_FAIL_IMMEDIATELY = 0x00000001


class Overlapped(ctypes.Structure):
    """Overlapped is required and used in LockFileEx and UnlockFileEx."""
    _fields_ = [('Internal', ctypes.wintypes.LPVOID),
                ('InternalHigh', ctypes.wintypes.LPVOID),
                ('Offset', ctypes.wintypes.DWORD),
                ('OffsetHigh', ctypes.wintypes.DWORD),
                ('Pointer', ctypes.wintypes.LPVOID),
                ('hEvent', ctypes.wintypes.HANDLE)]


# https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
CreateFileW = ctypes.windll.kernel32.CreateFileW
CreateFileW.argtypes = [
    ctypes.wintypes.LPCWSTR,  # lpFileName
    ctypes.wintypes.DWORD,  # dwDesiredAccess
    ctypes.wintypes.DWORD,  # dwShareMode
    ctypes.wintypes.LPVOID,  # lpSecurityAttributes
    ctypes.wintypes.DWORD,  # dwCreationDisposition
    ctypes.wintypes.DWORD,  # dwFlagsAndAttributes
    ctypes.wintypes.LPVOID,  # hTemplateFile
]
CreateFileW.restype = ctypes.wintypes.HANDLE

# https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
CloseHandle = ctypes.windll.kernel32.CloseHandle
CloseHandle.argtypes = [
    ctypes.wintypes.HANDLE,  # hFile
]
CloseHandle.restype = ctypes.wintypes.BOOL

# https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfileex
LockFileEx = ctypes.windll.kernel32.LockFileEx
LockFileEx.argtypes = [
    ctypes.wintypes.HANDLE,  # hFile
    ctypes.wintypes.DWORD,  # dwFlags
    ctypes.wintypes.DWORD,  # dwReserved
    ctypes.wintypes.DWORD,  # nNumberOfBytesToLockLow
    ctypes.wintypes.DWORD,  # nNumberOfBytesToLockHigh
    ctypes.POINTER(Overlapped),  # lpOverlapped
]
LockFileEx.restype = ctypes.wintypes.BOOL

# Commonly used functions are listed here so callers don't need to import
# ctypes.
GetLastError = ctypes.GetLastError
Handle = ctypes.wintypes.HANDLE
