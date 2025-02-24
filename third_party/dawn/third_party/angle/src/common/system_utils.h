//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils.h: declaration of OS-specific utility functions

#ifndef COMMON_SYSTEM_UTILS_H_
#define COMMON_SYSTEM_UTILS_H_

#include "common/Optional.h"
#include "common/angleutils.h"

#include <functional>
#include <string>
#include <string_view>

namespace angle
{
std::string GetExecutableName();
std::string GetExecutablePath();
std::string GetExecutableDirectory();
std::string GetModuleDirectory();
const char *GetSharedLibraryExtension();
const char *GetExecutableExtension();
char GetPathSeparator();
Optional<std::string> GetCWD();
bool SetCWD(const char *dirName);
bool SetEnvironmentVar(const char *variableName, const char *value);
bool UnsetEnvironmentVar(const char *variableName);
bool GetBoolEnvironmentVar(const char *variableName);
std::string GetEnvironmentVar(const char *variableName);
std::string GetEnvironmentVarOrUnCachedAndroidProperty(const char *variableName,
                                                       const char *propertyName);
std::string GetAndSetEnvironmentVarOrUnCachedAndroidProperty(const char *variableName,
                                                             const char *propertyName);
std::string GetEnvironmentVarOrAndroidProperty(const char *variableName, const char *propertyName);
const char *GetPathSeparatorForEnvironmentVar();
bool PrependPathToEnvironmentVar(const char *variableName, const char *path);
bool IsDirectory(const char *filename);
bool IsFullPath(std::string dirName);
bool CreateDirectories(const std::string &path);
void MakeForwardSlashThePathSeparator(std::string &path);
std::string GetRootDirectory();
std::string ConcatenatePath(std::string first, std::string second);

Optional<std::string> GetTempDirectory();
Optional<std::string> CreateTemporaryFileInDirectory(const std::string &directory);
Optional<std::string> CreateTemporaryFile();

#if defined(ANGLE_PLATFORM_POSIX)
// Same as CreateTemporaryFileInDirectory(), but allows for supplying an extension.
Optional<std::string> CreateTemporaryFileInDirectoryWithExtension(const std::string &directory,
                                                                  const std::string &extension);
#endif

// Get absolute time in seconds.  Use this function to get an absolute time with an unknown origin.
double GetCurrentSystemTime();
// Get CPU time for current process in seconds.
double GetCurrentProcessCpuTime();

// Unique thread id (std::this_thread::get_id() gets recycled!)
uint64_t GetCurrentThreadUniqueId();
// Fast function to get thread id when performance is critical (may be recycled).
// On Android 7-8x faster than GetCurrentThreadUniqueId().
ThreadId GetCurrentThreadId();
// Returns id that does not represent a thread.
ThreadId InvalidThreadId();

// Run an application and get the output.  Gets a nullptr-terminated set of args to execute the
// application with, and returns the stdout and stderr outputs as well as the exit code.
//
// Pass nullptr for stdoutOut/stderrOut if you don't need to capture. exitCodeOut is required.
//
// Returns false if it fails to actually execute the application.
bool RunApp(const std::vector<const char *> &args,
            std::string *stdoutOut,
            std::string *stderrOut,
            int *exitCodeOut);

// Use SYSTEM_DIR to bypass loading ANGLE libraries with the same name as system DLLS
// (e.g. opengl32.dll)
enum class SearchType
{
    // Try to find the library in the same directory as the current module
    ModuleDir,
    // Load the library from the system directories
    SystemDir,
    // Get a reference to an already loaded shared library.
    AlreadyLoaded,
};

void *OpenSystemLibrary(const char *libraryName, SearchType searchType);
void *OpenSystemLibraryWithExtension(const char *libraryName, SearchType searchType);
void *OpenSystemLibraryAndGetError(const char *libraryName,
                                   SearchType searchType,
                                   std::string *errorOut);
void *OpenSystemLibraryWithExtensionAndGetError(const char *libraryName,
                                                SearchType searchType,
                                                std::string *errorOut);

void *GetLibrarySymbol(void *libraryHandle, const char *symbolName);
std::string GetLibraryPath(void *libraryHandle);
void CloseSystemLibrary(void *libraryHandle);

class Library : angle::NonCopyable
{
  public:
    Library() {}
    Library(void *libraryHandle) : mLibraryHandle(libraryHandle) {}
    ~Library() { close(); }

    [[nodiscard]] bool open(const char *libraryName, SearchType searchType)
    {
        close();
        mLibraryHandle = OpenSystemLibrary(libraryName, searchType);
        return mLibraryHandle != nullptr;
    }

    [[nodiscard]] bool openWithExtension(const char *libraryName, SearchType searchType)
    {
        close();
        mLibraryHandle = OpenSystemLibraryWithExtension(libraryName, searchType);
        return mLibraryHandle != nullptr;
    }

    [[nodiscard]] bool openAndGetError(const char *libraryName,
                                       SearchType searchType,
                                       std::string *errorOut)
    {
        close();
        mLibraryHandle = OpenSystemLibraryAndGetError(libraryName, searchType, errorOut);
        return mLibraryHandle != nullptr;
    }

    [[nodiscard]] bool openWithExtensionAndGetError(const char *libraryName,
                                                    SearchType searchType,
                                                    std::string *errorOut)
    {
        close();
        mLibraryHandle =
            OpenSystemLibraryWithExtensionAndGetError(libraryName, searchType, errorOut);
        return mLibraryHandle != nullptr;
    }

    void close()
    {
        if (mLibraryHandle)
        {
            CloseSystemLibrary(mLibraryHandle);
            mLibraryHandle = nullptr;
        }
    }

    void *getSymbol(const char *symbolName) { return GetLibrarySymbol(mLibraryHandle, symbolName); }

    void *getNative() const { return mLibraryHandle; }

    std::string getPath() const { return GetLibraryPath(mLibraryHandle); }

    template <typename FuncT>
    void getAs(const char *symbolName, FuncT *funcOut)
    {
        *funcOut = reinterpret_cast<FuncT>(getSymbol(symbolName));
    }

  private:
    void *mLibraryHandle = nullptr;
};

Library *OpenSharedLibrary(const char *libraryName, SearchType searchType);
Library *OpenSharedLibraryWithExtension(const char *libraryName, SearchType searchType);
Library *OpenSharedLibraryAndGetError(const char *libraryName,
                                      SearchType searchType,
                                      std::string *errorOut);
Library *OpenSharedLibraryWithExtensionAndGetError(const char *libraryName,
                                                   SearchType searchType,
                                                   std::string *errorOut);

// Returns true if the process is currently being debugged.
bool IsDebuggerAttached();

// Calls system APIs to break into the debugger.
void BreakDebugger();

uint64_t GetProcessMemoryUsageKB();

bool ProtectMemory(uintptr_t start, size_t size);
bool UnprotectMemory(uintptr_t start, size_t size);

size_t GetPageSize();

// Return type of the PageFaultCallback
enum class PageFaultHandlerRangeType
{
    // The memory address was known by the page fault handler
    InRange,
    // The memory address was not in the page fault handler's range
    // and the signal will be forwarded to the default page handler.
    OutOfRange,
};

using PageFaultCallback = std::function<PageFaultHandlerRangeType(uintptr_t)>;

class PageFaultHandler : angle::NonCopyable
{
  public:
    PageFaultHandler(PageFaultCallback callback);
    virtual ~PageFaultHandler();

    // Registers OS level page fault handler for memory protection signals
    // and enables reception on PageFaultCallback
    virtual bool enable() = 0;

    // Unregisters OS level page fault handler and deactivates PageFaultCallback
    virtual bool disable() = 0;

  protected:
    PageFaultCallback mCallback;
};

// Creates single instance page fault handler
PageFaultHandler *CreatePageFaultHandler(PageFaultCallback callback);

#ifdef ANGLE_PLATFORM_WINDOWS
// Convert an UTF-16 wstring to an UTF-8 string.
std::string Narrow(const std::wstring_view &utf16);

// Convert an UTF-8 string to an UTF-16 wstring.
std::wstring Widen(const std::string_view &utf8);
#endif

std::string StripFilenameFromPath(const std::string &path);

#if defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_WINDOWS)
// Use C++ thread_local which is about 2x faster than std::this_thread::get_id()
ANGLE_INLINE ThreadId GetCurrentThreadId()
{
    thread_local int tls;
    return static_cast<ThreadId>(reinterpret_cast<uintptr_t>(&tls));
}
ANGLE_INLINE ThreadId InvalidThreadId()
{
    return -1;
}
#else
// Default. Fastest on Android (about the same as `pthread_self` and a bit faster then `gettid`).
ANGLE_INLINE ThreadId GetCurrentThreadId()
{
    return std::this_thread::get_id();
}
ANGLE_INLINE ThreadId InvalidThreadId()
{
    return ThreadId();
}
#endif

void SetCurrentThreadName(const char *name);
}  // namespace angle

#endif  // COMMON_SYSTEM_UTILS_H_
