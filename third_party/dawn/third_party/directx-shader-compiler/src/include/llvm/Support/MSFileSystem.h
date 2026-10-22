///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MSFileSystem.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides error code values for the DirectX compiler.                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef LLVM_SUPPORT_MSFILESYSTEM_H
#define LLVM_SUPPORT_MSFILESYSTEM_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// MSFileSystem interface.
struct stat;

namespace llvm {
namespace sys {
namespace fs {

class MSFileSystem {
public:
  virtual ~MSFileSystem(){};
  virtual BOOL FindNextFileW(HANDLE hFindFile,
                             LPWIN32_FIND_DATAW lpFindFileData) throw() = 0;
  virtual HANDLE FindFirstFileW(LPCWSTR lpFileName,
                                LPWIN32_FIND_DATAW lpFindFileData) throw() = 0;
  virtual void FindClose(HANDLE findHandle) throw() = 0;
  virtual HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                             DWORD dwShareMode, DWORD dwCreationDisposition,
                             DWORD dwFlagsAndAttributes) throw() = 0;
  virtual BOOL SetFileTime(HANDLE hFile, const FILETIME *lpCreationTime,
                           const FILETIME *lpLastAccessTime,
                           const FILETIME *lpLastWriteTime) throw() = 0;
  virtual BOOL GetFileInformationByHandle(
      HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation) throw() = 0;
  virtual DWORD GetFileType(HANDLE hFile) throw() = 0;
  virtual BOOL CreateHardLinkW(LPCWSTR lpFileName,
                               LPCWSTR lpExistingFileName) throw() = 0;
  virtual BOOL MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,
                           DWORD dwFlags) throw() = 0;
  virtual DWORD GetFileAttributesW(LPCWSTR lpFileName) throw() = 0;
  virtual BOOL CloseHandle(HANDLE hObject) throw() = 0;
  virtual BOOL DeleteFileW(LPCWSTR lpFileName) throw() = 0;
  virtual BOOL RemoveDirectoryW(LPCWSTR lpFileName) throw() = 0;
  virtual BOOL CreateDirectoryW(LPCWSTR lpPathName) throw() = 0;
  virtual DWORD GetCurrentDirectoryW(DWORD nBufferLength,
                                     LPWSTR lpBuffer) throw() = 0;
  virtual DWORD GetMainModuleFileNameW(LPWSTR lpFilename,
                                       DWORD nSize) throw() = 0;
  virtual DWORD GetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer) throw() = 0;
  virtual BOOLEAN CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName,
                                      LPCWSTR lpTargetFileName,
                                      DWORD dwFlags) throw() = 0;
  virtual bool SupportsCreateSymbolicLink() throw() = 0;
  virtual BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer,
                        DWORD nNumberOfBytesToRead,
                        LPDWORD lpNumberOfBytesRead) throw() = 0;
  virtual HANDLE CreateFileMappingW(HANDLE hFile, DWORD flProtect,
                                    DWORD dwMaximumSizeHigh,
                                    DWORD dwMaximumSizeLow) throw() = 0;
  virtual LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
                               DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                               SIZE_T dwNumberOfBytesToMap) throw() = 0;
  virtual BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) throw() = 0;

  // Console APIs.
  virtual bool FileDescriptorIsDisplayed(int fd) throw() = 0;
  virtual unsigned GetColumnCount(DWORD nStdHandle) throw() = 0;
  virtual unsigned GetConsoleOutputTextAttributes() throw() = 0;
  virtual void SetConsoleOutputTextAttributes(unsigned) throw() = 0;
  virtual void ResetConsoleOutputTextAttributes() throw() = 0;

  // CRT APIs.
  virtual int open_osfhandle(intptr_t osfhandle, int flags) throw() = 0;
  virtual intptr_t get_osfhandle(int fd) throw() = 0;
  virtual int close(int fd) throw() = 0;
  virtual long lseek(int fd, long offset, int origin) throw() = 0;
  virtual int setmode(int fd, int mode) throw() = 0;
  virtual errno_t resize_file(LPCWSTR path, uint64_t size) throw() = 0;
  virtual int Read(int fd, void *buffer, unsigned int count) throw() = 0;
  virtual int Write(int fd, const void *buffer, unsigned int count) throw() = 0;

  // Unix interface
#ifndef _WIN32
  virtual int Open(const char *lpFileName, int flags,
                   mode_t mode = 0) throw() = 0;
  virtual int Stat(const char *lpFileName, struct stat *Status) throw() = 0;
  virtual int Fstat(int FD, struct stat *Status) throw() = 0;
#endif
};

} // end namespace fs
} // end namespace sys
} // end namespace llvm

/// <summary>Creates a Win32/CRT-based implementation with full fidelity for a
/// console program.</summary> <remarks>This requires the LLVM MS Support
/// library to be linked in.</remarks>
HRESULT
CreateMSFileSystemForDisk(::llvm::sys::fs::MSFileSystem **pResult) throw();

struct IUnknown;

/// <summary>Creates an implementation based on IDxcSystemAccess.</summary>
HRESULT
CreateMSFileSystemForIface(IUnknown *pService,
                           ::llvm::sys::fs::MSFileSystem **pResult) throw();

#endif // LLVM_SUPPORT_MSFILESYSTEM_H
