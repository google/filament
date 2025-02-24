///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcfilesystem.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides helper file system for dxcompiler.                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include "dxcutil.h"
#include "llvm/Support/raw_ostream.h"

#include "dxc/Support/Path.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/dxcfilesystem.h"
#include "clang/Frontend/CompilerInstance.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace llvm;
using namespace hlsl;

// DxcArgsFileSystem
namespace {

#if defined(_MSC_VER)
#include <io.h>
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
#endif

#ifdef _WIN32
#ifndef NDEBUG

// This should be improved with global enabled mask rather than a compile-time
// mask.
#define DXTRACE_MASK_ENABLED 0
#define DXTRACE_MASK_APIFS 1
#define DXTRACE_ENABLED(subsystem) (DXTRACE_MASK_ENABLED & subsystem)

// DXTRACE_FMT formats a debugger trace message if DXTRACE_MASK allows it.
#define DXTRACE_FMT(subsystem, fmt, ...)                                       \
  do {                                                                         \
    if (DXTRACE_ENABLED(subsystem))                                            \
      OutputDebugFormatA(fmt, __VA_ARGS__);                                    \
  } while (0)
/// DXTRACE_FMT_APIFS is used by the API-based virtual filesystem.
#define DXTRACE_FMT_APIFS(fmt, ...)                                            \
  DXTRACE_FMT(DXTRACE_MASK_APIFS, fmt, __VA_ARGS__)

#else

#define DXTRACE_FMT_APIFS(...)

#endif // NDEBUG
#else  // _WIN32
#define DXTRACE_FMT_APIFS(...)
#endif // _WIN32

enum class HandleKind { Special = 0, File = 1, FileDir = 2, SearchDir = 3 };
enum class SpecialValue {
  Unknown = 0,
  StdOut = 1,
  StdErr = 2,
  Source = 3,
  Output = 4
};

// We use 10 bits for Offset to support MaxIncludedFiles include files,
// and we use 16 bits for Length to support nearly arbitrary path length.
struct HandleBits {
  unsigned Offset : 10;
  unsigned Length : 16;
  unsigned Kind : 4;
};
struct DxcArgsHandle {
  DxcArgsHandle(HANDLE h) : Handle(h) {}
  DxcArgsHandle(unsigned fileIndex) {
    Handle = 0;
    Bits.Offset = fileIndex;
    Bits.Length = 0;
    Bits.Kind = (unsigned)HandleKind::File;
  }
  DxcArgsHandle(HandleKind HK, unsigned fileIndex, unsigned dirLength) {
    Handle = 0;
    Bits.Offset = fileIndex;
    Bits.Length = dirLength;
    Bits.Kind = (unsigned)HK;
  }
  DxcArgsHandle(SpecialValue V) {
    Handle = 0;
    Bits.Offset = (unsigned)V;
    Bits.Length = 0;
    Bits.Kind = (unsigned)HandleKind::Special;
    ;
  }

  union {
    HANDLE Handle;
    HandleBits Bits;
  };
  bool operator==(const DxcArgsHandle &Other) { return Handle == Other.Handle; }
  HandleKind GetKind() const { return (HandleKind)Bits.Kind; }
  bool IsFileKind() const { return GetKind() == HandleKind::File; }
  bool IsSpecialUnknown() const { return Handle == 0; }
  bool IsDirHandle() const {
    return GetKind() == HandleKind::FileDir ||
           GetKind() == HandleKind::SearchDir;
  }
  bool IsStdHandle() const {
    return GetKind() == HandleKind::Special &&
           (GetSpecialValue() == SpecialValue::StdErr ||
            GetSpecialValue() == SpecialValue::StdOut);
  }
  unsigned GetFileIndex() const {
    DXASSERT_NOMSG(IsFileKind());
    return Bits.Offset;
  }
  SpecialValue GetSpecialValue() const {
    DXASSERT_NOMSG(GetKind() == HandleKind::Special);
    return (SpecialValue)Bits.Offset;
  }
  unsigned Length() const { return Bits.Length; }
};

static_assert(sizeof(DxcArgsHandle) == sizeof(HANDLE),
              "else can't transparently typecast");

const DxcArgsHandle UnknownHandle(SpecialValue::Unknown);
const DxcArgsHandle StdOutHandle(SpecialValue::StdOut);
const DxcArgsHandle StdErrHandle(SpecialValue::StdErr);
const DxcArgsHandle OutputHandle(SpecialValue::Output);

/// Max number of included files (1:1 to their directories) or search
/// directories. If programs include more than a handful, DxcArgsFileSystem will
/// need to do better than linear scans. If this is fired,
/// ERROR_OUT_OF_STRUCTURES will be returned by an attempt to open a file.
static const size_t MaxIncludedFiles = 1000;

} // namespace

namespace dxcutil {

void MakeAbsoluteOrCurDirRelativeW(LPCWSTR &Path, std::wstring &PathStorage) {
  if (hlsl::IsAbsoluteOrCurDirRelativeW(Path)) {
    return;
  } else {
    PathStorage = L"./";
    PathStorage += Path;
    Path = PathStorage.c_str();
  }
}

/// File system based on API arguments. Support being added incrementally.
///
/// DxcArgsFileSystem emulates a file system to clang/llvm based on API
/// arguments. It can block certain functionality (like picking up the current
/// directory), while adding other (like supporting an app's in-memory
/// files through an IDxcIncludeHandler).
///
/// stdin/stdout/stderr are registered especially (given that they have a
/// special role in llvm::ins/outs/errs and are defaults to various operations,
/// it's not unexpected). The direct user of DxcArgsFileSystem can also register
/// streams to capture output for specific files.
///
/// Support for IDxcIncludeHandler is somewhat tricky because the API is very
/// minimal, to allow simple implementations, but that puts this class in the
/// position of brokering between llvm/clang existing files (which probe for
/// files and directories in various patterns), and this simpler handler.
/// The current approach is to minimize changes in llvm/clang and work around
/// the absence of directory support in IDxcIncludeHandler by assuming all
/// included paths already exist (the handler may reject those paths later on),
/// and always querying for a file before its parent directory (so we can
/// disambiguate between one or the other).
class DxcArgsFileSystemImpl : public DxcArgsFileSystem {
private:
  CComPtr<IDxcBlobUtf8> m_pSource;
  LPCWSTR m_pSourceName;
  std::wstring m_pAbsSourceName; // absolute (or '.'-relative) source name
  CComPtr<IStream> m_pSourceStream;
  CComPtr<IStream> m_pOutputStream;
  CComPtr<AbstractMemoryStream> m_pStdOutStream;
  CComPtr<AbstractMemoryStream> m_pStdErrStream;
  LPCWSTR m_pOutputStreamName;
  std::wstring m_pAbsOutputStreamName;
  CComPtr<IDxcIncludeHandler> m_includeLoader;
  std::vector<std::wstring> m_searchEntries;
  bool m_bDisplayIncludeProcess;
  UINT32 m_DefaultCodePage;

  // Some constraints of the current design: opening the same file twice
  // will return the same handle/structure, and thus the same file pointer.
  struct IncludedFile {
    CComPtr<IDxcBlobUtf8> Blob;
    CComPtr<IStream> BlobStream;
    std::wstring Name;
    IncludedFile(std::wstring &&name, IDxcBlobUtf8 *pBlob, IStream *pStream)
        : Blob(pBlob), BlobStream(pStream), Name(name) {}
  };
  llvm::SmallVector<IncludedFile, 4> m_includedFiles;

  static bool IsDirOf(LPCWSTR lpDir, size_t dirLen,
                      const std::wstring &fileName) {
    if (fileName.size() <= dirLen)
      return false;
    if (0 != wcsncmp(lpDir, fileName.data(), dirLen))
      return false;

    // Prefix matches, c:\\ to c:\\foo.hlsl or ./bar to ./bar/file.hlsl
    // Ensure there are no additional characters, don't match ./ba if ./bar.hlsl
    // exists
    if (lpDir[dirLen - 1] == '\\' || lpDir[dirLen - 1] == '/') {
      // The file name was already terminated in a separator.
      return true;
    }

    return fileName.data()[dirLen] == '\\' || fileName.data()[dirLen] == '/';
  }

  static bool IsDirPrefixOrSame(LPCWSTR lpDir, size_t dirLen,
                                const std::wstring &path) {
    if (0 == wcscmp(lpDir, path.c_str()))
      return true;
    return IsDirOf(lpDir, dirLen, path);
  }

  HANDLE TryFindDirHandle(LPCWSTR lpDir) const {
    size_t dirLen = wcslen(lpDir);
    for (size_t i = 0; i < m_includedFiles.size(); ++i) {
      const std::wstring &fileName = m_includedFiles[i].Name;
      if (IsDirOf(lpDir, dirLen, fileName)) {
        return DxcArgsHandle(HandleKind::FileDir, i, dirLen).Handle;
      }
    }
    for (size_t i = 0; i < m_searchEntries.size(); ++i) {
      if (IsDirPrefixOrSame(lpDir, dirLen, m_searchEntries[i])) {
        return DxcArgsHandle(HandleKind::SearchDir, i, dirLen).Handle;
      }
    }
    return INVALID_HANDLE_VALUE;
  }
  DWORD TryFindOrOpen(LPCWSTR lpFileName, size_t &index) {
    for (size_t i = 0; i < m_includedFiles.size(); ++i) {
      if (0 == wcscmp(lpFileName, m_includedFiles[i].Name.data())) {
        index = i;
        return ERROR_SUCCESS;
      }
    }

    if (m_includeLoader.p != nullptr) {
      if (m_includedFiles.size() == MaxIncludedFiles) {
        return ERROR_OUT_OF_STRUCTURES;
      }

      CComPtr<::IDxcBlob> fileBlob;

      std::wstring NormalizedFileName = hlsl::NormalizePathW(lpFileName);
      HRESULT hr =
          m_includeLoader->LoadSource(NormalizedFileName.c_str(), &fileBlob);
      if (FAILED(hr)) {
        return ERROR_UNHANDLED_EXCEPTION;
      }
      if (fileBlob.p != nullptr) {
        CComPtr<IDxcBlobUtf8> fileBlobUtf8;
        if (FAILED(hlsl::DxcGetBlobAsUtf8(fileBlob, DxcGetThreadMallocNoRef(),
                                          &fileBlobUtf8, m_DefaultCodePage))) {
          return ERROR_UNHANDLED_EXCEPTION;
        }
        CComPtr<IStream> fileStream;
        if (FAILED(hlsl::CreateReadOnlyBlobStream(fileBlobUtf8, &fileStream))) {
          return ERROR_UNHANDLED_EXCEPTION;
        }
        m_includedFiles.emplace_back(std::wstring(lpFileName), fileBlobUtf8,
                                     fileStream);
        index = m_includedFiles.size() - 1;

        if (m_bDisplayIncludeProcess) {
          std::string openFileStr;
          raw_string_ostream s(openFileStr);
          std::string fileName = Unicode::WideToUTF8StringOrThrow(lpFileName);
          s << "Opening file [" << fileName << "], stack top [" << (index - 1)
            << "]\n";
          s.flush();
          ULONG cbWritten;
          IFT(m_pStdOutStream->Write(openFileStr.c_str(), openFileStr.size(),
                                     &cbWritten));
        }
        return ERROR_SUCCESS;
      }
    }
    return ERROR_NOT_FOUND;
  }
  static HANDLE IncludedFileIndexToHandle(size_t index) {
    return DxcArgsHandle(index).Handle;
  }
  bool IsKnownHandle(HANDLE h) const {
    return !DxcArgsHandle(h).IsSpecialUnknown();
  }
  IncludedFile &HandleToIncludedFile(HANDLE handle) {
    DxcArgsHandle argsHandle(handle);
    DXASSERT_NOMSG(argsHandle.GetFileIndex() < m_includedFiles.size());
    return m_includedFiles[argsHandle.GetFileIndex()];
  }

public:
  DxcArgsFileSystemImpl(IDxcBlobUtf8 *pSource, LPCWSTR pSourceName,
                        IDxcIncludeHandler *pHandler, UINT32 defaultCodePage)
      : m_pSource(pSource), m_pSourceName(pSourceName),
        m_pOutputStreamName(nullptr), m_includeLoader(pHandler),
        m_bDisplayIncludeProcess(false), m_DefaultCodePage(defaultCodePage) {
    MakeAbsoluteOrCurDirRelativeW(m_pSourceName, m_pAbsSourceName);
    IFT(CreateReadOnlyBlobStream(m_pSource, &m_pSourceStream));
    m_includedFiles.push_back(
        IncludedFile(std::wstring(m_pSourceName), m_pSource, m_pSourceStream));
  }
  void EnableDisplayIncludeProcess() override {
    m_bDisplayIncludeProcess = true;
  }
  void WriteStdErrToStream(raw_string_ostream &s) override {
    s.write((char *)m_pStdErrStream->GetPtr(), m_pStdErrStream->GetPtrSize());
    s.flush();
  }
  void WriteStdOutToStream(raw_string_ostream &s) override {
    s.write((char *)m_pStdOutStream->GetPtr(), m_pStdOutStream->GetPtrSize());
    s.flush();
  }
  HRESULT CreateStdStreams(IMalloc *pMalloc) override {
    DXASSERT(m_pStdOutStream == nullptr, "else already created");
    CreateMemoryStream(pMalloc, &m_pStdOutStream);
    CreateMemoryStream(pMalloc, &m_pStdErrStream);
    if (m_pStdOutStream == nullptr || m_pStdErrStream == nullptr) {
      return E_OUTOFMEMORY;
    }
    return S_OK;
  }

  void GetStreamForFD(int fd, IStream **ppResult) {
    return GetStreamForHandle(HandleFromFD(fd), ppResult);
  }
  void GetStreamForHandle(HANDLE handle, IStream **ppResult) {
    CComPtr<IStream> stream;
    DxcArgsHandle argsHandle(handle);
    if (argsHandle == OutputHandle) {
      stream = m_pOutputStream;
    } else if (argsHandle == StdOutHandle) {
      stream = m_pStdOutStream;
    } else if (argsHandle == StdErrHandle) {
      stream = m_pStdErrStream;
    } else if (argsHandle.GetKind() == HandleKind::File) {
      stream = HandleToIncludedFile(handle).BlobStream;
    }
    *ppResult = stream.Detach();
  }

  void GetStdOutputHandleStream(IStream **ppResultStream) override {
    return GetStreamForHandle(StdOutHandle.Handle, ppResultStream);
  }

  void GetStdErrorHandleStream(IStream **ppResultStream) override {
    return GetStreamForHandle(StdErrHandle.Handle, ppResultStream);
  }

  void SetupForCompilerInstance(clang::CompilerInstance &compiler) override {
    DXASSERT(m_searchEntries.size() == 0,
             "else compiler instance being set twice");
    // Turn these into UTF-16 to avoid converting later, and ensure they
    // are fully-qualified or relative to the current directory.
    const std::vector<clang::HeaderSearchOptions::Entry> &entries =
        compiler.getHeaderSearchOpts().UserEntries;
    if (entries.size() > MaxIncludedFiles) {
      throw hlsl::Exception(HRESULT_FROM_WIN32(ERROR_OUT_OF_STRUCTURES));
    }
    for (unsigned i = 0, e = entries.size(); i != e; ++i) {
      const clang::HeaderSearchOptions::Entry &E = entries[i];
      if (dxcutil::IsAbsoluteOrCurDirRelative(E.Path.c_str())) {
        m_searchEntries.emplace_back(
            Unicode::UTF8ToWideStringOrThrow(E.Path.c_str()));
      } else {
        std::wstring ws(L"./");
        ws += Unicode::UTF8ToWideStringOrThrow(E.Path.c_str());
        m_searchEntries.emplace_back(std::move(ws));
      }
    }
  }

  HRESULT RegisterOutputStream(LPCWSTR pName, IStream *pStream) override {
    DXASSERT(m_pOutputStream.p == nullptr, "else multiple outputs registered");
    m_pOutputStream = pStream;
    m_pOutputStreamName = pName;
    MakeAbsoluteOrCurDirRelativeW(m_pOutputStreamName, m_pAbsOutputStreamName);
    return S_OK;
  }

  HRESULT UnRegisterOutputStream() override {
    m_pOutputStream.Detach();
    m_pOutputStream = nullptr;
    return S_OK;
  }

  ~DxcArgsFileSystemImpl() override{};
  BOOL FindNextFileW(HANDLE hFindFile,
                     LPWIN32_FIND_DATAW lpFindFileData) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }

  HANDLE FindFirstFileW(LPCWSTR lpFileName,
                        LPWIN32_FIND_DATAW lpFindFileData) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }

  void FindClose(HANDLE findHandle) throw() override { __debugbreak(); }

  HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                     DWORD dwShareMode, DWORD dwCreationDisposition,
                     DWORD dwFlagsAndAttributes) throw() override {
    DXTRACE_FMT_APIFS("DxcArgsFileSystem::CreateFileW %S\n", lpFileName);
    DWORD findError;
    {
      std::wstring FileNameStore; // The destructor might release and set
                                  // LastError to success.
      MakeAbsoluteOrCurDirRelativeW(lpFileName, FileNameStore);

      // Check for a match to the output file.
      if (m_pOutputStreamName != nullptr &&
          0 == wcscmp(lpFileName, m_pOutputStreamName)) {
        return OutputHandle.Handle;
      }

      HANDLE dirHandle = TryFindDirHandle(lpFileName);
      if (dirHandle != INVALID_HANDLE_VALUE) {
        return dirHandle;
      }

      size_t includedIndex;
      findError = TryFindOrOpen(lpFileName, includedIndex);
      if (findError == ERROR_SUCCESS) {
        return IncludedFileIndexToHandle(includedIndex);
      }
    }

    SetLastError(findError);
    return INVALID_HANDLE_VALUE;
  }

  BOOL SetFileTime(HANDLE hFile, const FILETIME *lpCreationTime,
                   const FILETIME *lpLastAccessTime,
                   const FILETIME *lpLastWriteTime) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }

  BOOL GetFileInformationByHandle(
      HANDLE hFile,
      LPBY_HANDLE_FILE_INFORMATION lpFileInformation) throw() override {
    DxcArgsHandle argsHandle(hFile);
    ZeroMemory(lpFileInformation, sizeof(*lpFileInformation));
    lpFileInformation->nFileIndexLow = (DWORD)(uintptr_t)hFile;
    if (argsHandle.IsFileKind()) {
      IncludedFile &file = HandleToIncludedFile(hFile);
      lpFileInformation->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
      lpFileInformation->nFileSizeLow = file.Blob->GetStringLength();
      return TRUE;
    }
    if (argsHandle == OutputHandle) {
      lpFileInformation->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
      STATSTG stat;
      HRESULT hr = m_pOutputStream->Stat(&stat, STATFLAG_NONAME);
      if (FAILED(hr)) {
        SetLastError(ERROR_IO_DEVICE);
        return FALSE;
      }
      lpFileInformation->nFileSizeLow = stat.cbSize.u.LowPart;
      return TRUE;
    } else if (argsHandle.IsDirHandle()) {
      lpFileInformation->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      lpFileInformation->nFileIndexHigh = 1;
      return TRUE;
    }
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }

  DWORD GetFileType(HANDLE hFile) throw() override {
    DxcArgsHandle argsHandle(hFile);
    if (argsHandle.IsStdHandle()) {
      return FILE_TYPE_CHAR;
    }
    // Every other known handle is of type disk.
    if (!argsHandle.IsSpecialUnknown()) {
      return FILE_TYPE_DISK;
    }

    SetLastError(ERROR_NOT_FOUND);
    return FILE_TYPE_UNKNOWN;
  }

  BOOL CreateHardLinkW(LPCWSTR lpFileName,
                       LPCWSTR lpExistingFileName) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  BOOL MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,
                   DWORD dwFlags) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  DWORD GetFileAttributesW(LPCWSTR lpFileName) throw() override {
    DXTRACE_FMT_APIFS("DxcArgsFileSystem::GetFileAttributesW %S\n", lpFileName);
    DWORD findError;
    {
      std::wstring FileNameStore; // The destructor might release and set
                                  // LastError to success.
      MakeAbsoluteOrCurDirRelativeW(lpFileName, FileNameStore);
      size_t sourceNameLen = wcslen(m_pSourceName);
      size_t fileNameLen = wcslen(lpFileName);

      // Check for a match to the source.
      if (fileNameLen == sourceNameLen) {
        if (0 == wcsncmp(m_pSourceName, lpFileName, fileNameLen)) {
          return FILE_ATTRIBUTE_NORMAL;
        }
      }

      // Check for a perfect match to the output.
      if (m_pOutputStreamName != nullptr &&
          0 == wcscmp(m_pOutputStreamName, lpFileName)) {
        return FILE_ATTRIBUTE_NORMAL;
      }

      if (TryFindDirHandle(lpFileName) != INVALID_HANDLE_VALUE) {
        return FILE_ATTRIBUTE_DIRECTORY;
      }

      size_t includedIndex;
      findError = TryFindOrOpen(lpFileName, includedIndex);
      if (findError == ERROR_SUCCESS) {
        return FILE_ATTRIBUTE_NORMAL;
      }
    }

    SetLastError(findError);
    return INVALID_FILE_ATTRIBUTES;
  }

  BOOL CloseHandle(HANDLE hObject) throw() override {
    // Not actually closing handle. Would allow improper usage, but simplifies
    // query/open/usage patterns.
    if (IsKnownHandle(hObject)) {
      return TRUE;
    }

    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }
  BOOL DeleteFileW(LPCWSTR lpFileName) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  BOOL RemoveDirectoryW(LPCWSTR lpFileName) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  BOOL CreateDirectoryW(LPCWSTR lpPathName) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  DWORD GetCurrentDirectoryW(DWORD nBufferLength,
                             LPWSTR lpBuffer) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  DWORD GetMainModuleFileNameW(LPWSTR lpFilename,
                               DWORD nSize) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  DWORD GetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  BOOLEAN CreateSymbolicLinkW(LPCWSTR lpSymlinkFileName,
                              LPCWSTR lpTargetFileName,
                              DWORD dwFlags) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  bool SupportsCreateSymbolicLink() throw() override { return false; }
  BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                LPDWORD lpNumberOfBytesRead) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }
  HANDLE CreateFileMappingW(HANDLE hFile, DWORD flProtect,
                            DWORD dwMaximumSizeHigh,
                            DWORD dwMaximumSizeLow) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return INVALID_HANDLE_VALUE;
  }
  LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
                       DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow,
                       SIZE_T dwNumberOfBytesToMap) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return nullptr;
  }
  BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) throw() override {
    SetLastError(ERROR_NOT_CAPABLE);
    return FALSE;
  }

  // Console APIs.
  bool FileDescriptorIsDisplayed(int fd) throw() override { return false; }
  unsigned GetColumnCount(DWORD nStdHandle) throw() override { return 80; }
  unsigned GetConsoleOutputTextAttributes() throw() override { return 0; }
  void SetConsoleOutputTextAttributes(unsigned) throw() override {
    __debugbreak();
  }
  void ResetConsoleOutputTextAttributes() throw() override { __debugbreak(); }

  // CRT APIs - handles and file numbers can be mapped directly.
  HANDLE HandleFromFD(int fd) const {
    if (fd == STDOUT_FILENO)
      return StdOutHandle.Handle;
    if (fd == STDERR_FILENO)
      return StdErrHandle.Handle;
    return (HANDLE)(uintptr_t)(fd);
  }
  int open_osfhandle(intptr_t osfhandle, int flags) throw() override {
    DxcArgsHandle H((HANDLE)osfhandle);
    if (H == StdOutHandle.Handle)
      return STDOUT_FILENO;
    if (H == StdErrHandle.Handle)
      return STDERR_FILENO;
    return (int)(intptr_t)H.Handle;
  }
  intptr_t get_osfhandle(int fd) throw() override {
    return (intptr_t)HandleFromFD(fd);
  }
  int close(int fd) throw() override { return 0; }
  long lseek(int fd, long offset, int origin) throw() override {
    CComPtr<IStream> stream;
    GetStreamForFD(fd, &stream);
    if (stream == nullptr) {
      errno = EBADF;
      return -1;
    }

    LARGE_INTEGER li;
    li.u.LowPart = offset;
    li.u.HighPart = 0;
    ULARGE_INTEGER newOffset;
    HRESULT hr = stream->Seek(li, origin, &newOffset);
    if (FAILED(hr)) {
      errno = EINVAL;
      return -1;
    }

    return newOffset.u.LowPart;
  }
  int setmode(int fd, int mode) throw() override { return 0; }
  errno_t resize_file(LPCWSTR path, uint64_t size) throw() override {
    return 0;
  }
  int Read(int fd, void *buffer, unsigned int count) throw() override {
    CComPtr<IStream> stream;
    GetStreamForFD(fd, &stream);
    if (stream == nullptr) {
      errno = EBADF;
      return -1;
    }

    ULONG cbRead;
    HRESULT hr = stream->Read(buffer, count, &cbRead);
    if (FAILED(hr)) {
      errno = EIO;
      return -1;
    }

    return (int)cbRead;
  }
  int Write(int fd, const void *buffer, unsigned int count) throw() override {
    CComPtr<IStream> stream;
    GetStreamForFD(fd, &stream);
    if (stream == nullptr) {
      errno = EBADF;
      return -1;
    }

#ifndef NDEBUG
    if (fd == STDERR_FILENO) {
      char *copyWithNull = new char[count + 1];
      strncpy(copyWithNull, (const char *)buffer, count);
      copyWithNull[count] = '\0';
      OutputDebugStringA(copyWithNull);
      delete[] copyWithNull;
    }
#endif

    ULONG written;
    HRESULT hr = stream->Write(buffer, count, &written);
    if (FAILED(hr)) {
      errno = EIO;
      return -1;
    }

    return (int)written;
  }
#ifndef _WIN32
  // Replicate Unix interfaces using DxcArgsFileSystemImpl

  int getStatus(HANDLE FileHandle, struct stat *Status) {
    if (FileHandle == INVALID_HANDLE_VALUE)
      return -1;

    memset(Status, 0, sizeof(*Status));
    switch (GetFileType(FileHandle)) {
    default:
      llvm_unreachable("Don't know anything about this file type");
    case FILE_TYPE_DISK:
      break;
    case FILE_TYPE_CHAR:
      Status->st_mode = S_IFCHR;
      return 0;
    case FILE_TYPE_PIPE:
      Status->st_mode = S_IFIFO;
      return 0;
    }

    BY_HANDLE_FILE_INFORMATION Info;
    if (!GetFileInformationByHandle(FileHandle, &Info))
      return -1;

    if (Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      Status->st_mode = S_IFDIR;
    else
      Status->st_mode = S_IFREG;

    Status->st_dev = Info.nFileIndexHigh;
    Status->st_ino = Info.nFileIndexLow;
    Status->st_mtime = Info.ftLastWriteTime.dwLowDateTime;
    Status->st_size = Info.nFileSizeLow;
    return 0;
  }

  virtual int Open(const char *lpFileName, int flags,
                   mode_t mode) throw() override {
    HANDLE H = CreateFileW(CA2W(lpFileName), GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL);
    if (H == INVALID_HANDLE_VALUE)
      return -1;
    int FD = open_osfhandle(intptr_t(H), 0);
    if (FD == -1)
      CloseHandle(H);
    return FD;
  }

  // fake my way toward as linux-y a file_status as I can get
  virtual int Stat(const char *lpFileName,
                   struct stat *Status) throw() override {
    CA2W fileName_wide(lpFileName);

    DWORD attr = GetFileAttributesW(fileName_wide);
    if (attr == INVALID_FILE_ATTRIBUTES)
      return -1;

    HANDLE H =
        CreateFileW(fileName_wide, 0, // Attributes only.
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);
    if (H == INVALID_HANDLE_VALUE)
      return -1;

    return getStatus(H, Status);
  }

  virtual int Fstat(int FD, struct stat *Status) throw() override {
    HANDLE FileHandle = reinterpret_cast<HANDLE>(get_osfhandle(FD));
    return getStatus(FileHandle, Status);
  }
#endif // _WIN32
};
} // namespace dxcutil

namespace dxcutil {

DxcArgsFileSystem *CreateDxcArgsFileSystem(IDxcBlobUtf8 *pSource,
                                           LPCWSTR pSourceName,
                                           IDxcIncludeHandler *pIncludeHandler,
                                           UINT32 defaultCodePage) {
  return new DxcArgsFileSystemImpl(pSource, pSourceName, pIncludeHandler,
                                   defaultCodePage);
}

} // namespace dxcutil
