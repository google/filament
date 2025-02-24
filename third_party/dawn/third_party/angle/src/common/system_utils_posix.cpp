//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_posix.cpp: Implementation of POSIX OS-specific functions.

#include "common/debug.h"
#include "system_utils.h"

#include <array>
#include <iostream>

#include <dlfcn.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common/string_utils.h"

#ifdef ANGLE_PLATFORM_FUCHSIA
#    include <zircon/process.h>
#    include <zircon/syscalls.h>
#else
#    include <sys/resource.h>
#endif

namespace angle
{

namespace
{
std::string GetModulePath(void *moduleOrSymbol)
{
    Dl_info dlInfo;
    if (dladdr(moduleOrSymbol, &dlInfo) == 0)
    {
        return "";
    }

#ifdef ANGLE_PLATFORM_LINUX
    // Chrome changes process title on Linux that causes dladdr returns wrong module
    // file name for executable binary, so return GetExecutablePath() if dli_fname
    // doesn't exist.
    struct stat buf;
    if (stat(dlInfo.dli_fname, &buf) != 0)
    {
        return GetExecutablePath();
    }
#endif

    return dlInfo.dli_fname;
}

void *OpenPosixLibrary(const std::string &fullPath, int extraFlags, std::string *errorOut)
{
    void *module = dlopen(fullPath.c_str(), RTLD_NOW | extraFlags);
    if (module)
    {
        if (errorOut)
        {
            *errorOut = fullPath;
        }
    }
    else if (errorOut)
    {
        *errorOut = "dlopen(";
        *errorOut += fullPath;
        *errorOut += ") failed with error: ";
        *errorOut += dlerror();
        struct stat sfile;
        if (-1 == stat(fullPath.c_str(), &sfile))
        {
            *errorOut += ", stat() call failed.";
        }
        else
        {
            *errorOut += ", stat() info: ";
            struct passwd *pwuser = getpwuid(sfile.st_uid);
            if (pwuser)
            {
                *errorOut += "owner: ";
                *errorOut += pwuser->pw_name;
                *errorOut += ", ";
            }
            struct group *grpnam = getgrgid(sfile.st_gid);
            if (grpnam)
            {
                *errorOut += "group: ";
                *errorOut += grpnam->gr_name;
                *errorOut += ", ";
            }
            *errorOut += "perms: ";
            *errorOut += std::to_string(sfile.st_mode);
            *errorOut += ", links: ";
            *errorOut += std::to_string(sfile.st_nlink);
            *errorOut += ", size: ";
            *errorOut += std::to_string(sfile.st_size);
        }
    }
    return module;
}
}  // namespace

Optional<std::string> GetCWD()
{
    std::array<char, 4096> pathBuf;
    char *result = getcwd(pathBuf.data(), pathBuf.size());
    if (result == nullptr)
    {
        return Optional<std::string>::Invalid();
    }
    return std::string(pathBuf.data());
}

bool SetCWD(const char *dirName)
{
    return (chdir(dirName) == 0);
}

bool UnsetEnvironmentVar(const char *variableName)
{
    return (unsetenv(variableName) == 0);
}

bool SetEnvironmentVar(const char *variableName, const char *value)
{
    return (setenv(variableName, value, 1) == 0);
}

std::string GetEnvironmentVar(const char *variableName)
{
    const char *value = getenv(variableName);
    return (value == nullptr ? std::string() : std::string(value));
}

const char *GetPathSeparatorForEnvironmentVar()
{
    return ":";
}

std::string GetModuleDirectoryAndGetError(std::string *errorOut)
{
    std::string directory;
    static int placeholderSymbol = 0;
    std::string moduleName       = GetModulePath(&placeholderSymbol);
    if (!moduleName.empty())
    {
        directory = moduleName.substr(0, moduleName.find_last_of('/') + 1);
    }

    // Ensure we return the full path to the module, not the relative path
    if (!IsFullPath(directory))
    {
        if (errorOut)
        {
            *errorOut += "Directory: '";
            *errorOut += directory;
            *errorOut += "' is not full path";
        }
        Optional<std::string> cwd = GetCWD();
        if (cwd.valid())
        {
            directory = ConcatenatePath(cwd.value(), directory);
            if (errorOut)
            {
                *errorOut += ", so it has been modified to: '";
                *errorOut += directory;
                *errorOut += "'. ";
            }
        }
        else if (errorOut)
        {
            *errorOut += " and getcwd was invalid. ";
        }
    }
    return directory;
}

std::string GetModuleDirectory()
{
    return GetModuleDirectoryAndGetError(nullptr);
}

void *OpenSystemLibraryWithExtensionAndGetError(const char *libraryName,
                                                SearchType searchType,
                                                std::string *errorOut)
{
    std::string directory;
    if (searchType == SearchType::ModuleDir)
    {
#if ANGLE_PLATFORM_IOS_FAMILY
        // On iOS, shared libraries must be loaded from within the app bundle.
        directory = GetExecutableDirectory() + "/Frameworks/";
#elif ANGLE_PLATFORM_FUCHSIA
        // On Fuchsia the dynamic loader always looks up libraries in /pkg/lib
        // and disallows loading of libraries via absolute paths.
        directory = "";
#else
        directory = GetModuleDirectoryAndGetError(errorOut);
#endif
    }

    int extraFlags = 0;
    if (searchType == SearchType::AlreadyLoaded)
    {
        extraFlags = RTLD_NOLOAD;
    }

    std::string fullPath = directory + libraryName;
    return OpenPosixLibrary(fullPath, extraFlags, errorOut);
}

void *GetLibrarySymbol(void *libraryHandle, const char *symbolName)
{
    if (!libraryHandle)
    {
        return nullptr;
    }

    return dlsym(libraryHandle, symbolName);
}

std::string GetLibraryPath(void *libraryHandle)
{
    if (!libraryHandle)
    {
        return "";
    }

    return GetModulePath(libraryHandle);
}

void CloseSystemLibrary(void *libraryHandle)
{
    if (libraryHandle)
    {
        dlclose(libraryHandle);
    }
}

bool IsDirectory(const char *filename)
{
    struct stat st;
    int result = stat(filename, &st);
    return result == 0 && ((st.st_mode & S_IFDIR) == S_IFDIR);
}

bool IsDebuggerAttached()
{
    // This could have a fuller implementation.
    // See https://cs.chromium.org/chromium/src/base/debug/debugger_posix.cc
    return false;
}

void BreakDebugger()
{
    // This could have a fuller implementation.
    // See https://cs.chromium.org/chromium/src/base/debug/debugger_posix.cc
    abort();
}

const char *GetExecutableExtension()
{
    return "";
}

char GetPathSeparator()
{
    return '/';
}

std::string GetRootDirectory()
{
    return "/";
}

bool CreateDirectories(const std::string &path)
{
    // First sanitize path so we can use "/" as universal path separator
    std::string sanitizedPath(path);
    MakeForwardSlashThePathSeparator(sanitizedPath);

    size_t pos = 0;
    do
    {
        pos = sanitizedPath.find("/", pos);
        std::string checkPath(sanitizedPath.substr(0, pos));
        if (!checkPath.empty() && !IsDirectory(checkPath.c_str()))
        {
            if (mkdir(checkPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
            {
                return false;
            }
        }
        if (pos == std::string::npos)
        {
            break;
        }
        ++pos;
    } while (true);
    return true;
}

void MakeForwardSlashThePathSeparator(std::string &path)
{
    // Nothing to do here for *nix side
    return;
}

Optional<std::string> GetTempDirectory()
{
    const char *tmp = getenv("TMPDIR");
    if (tmp != nullptr)
    {
        return std::string(tmp);
    }

#if defined(ANGLE_PLATFORM_ANDROID)
    // Not used right now in the ANGLE test runner.
    // return PathService::Get(DIR_CACHE, path);
    return Optional<std::string>::Invalid();
#else
    return std::string("/tmp");
#endif
}

Optional<std::string> CreateTemporaryFileInDirectory(const std::string &directory)
{
    return CreateTemporaryFileInDirectoryWithExtension(directory, std::string());
}

Optional<std::string> CreateTemporaryFileInDirectoryWithExtension(const std::string &directory,
                                                                  const std::string &extension)
{
    std::string tempFileTemplate = directory + "/.angle.XXXXXX" + extension;

    int fd = mkstemps(&tempFileTemplate[0], static_cast<int>(extension.size()));
    close(fd);

    if (fd != -1)
    {
        return tempFileTemplate;
    }

    return Optional<std::string>::Invalid();
}

double GetCurrentProcessCpuTime()
{
#ifdef ANGLE_PLATFORM_FUCHSIA
    static zx_handle_t me = zx_process_self();
    zx_info_task_runtime_t task_runtime;
    zx_object_get_info(me, ZX_INFO_TASK_RUNTIME, &task_runtime, sizeof(task_runtime), nullptr,
                       nullptr);
    return static_cast<double>(task_runtime.cpu_time) * 1e-9;
#else
    // We could also have used /proc/stat, but that requires us to read the
    // filesystem and convert from jiffies. /proc/stat also relies on jiffies
    // (lower resolution) while getrusage can potentially use a sched_clock()
    // underneath that has higher resolution.
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double userTime   = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec * 1e-6;
    double systemTime = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec * 1e-6;
    return userTime + systemTime;
#endif
}

namespace
{
bool SetMemoryProtection(uintptr_t start, size_t size, int protections)
{
    int ret = mprotect(reinterpret_cast<void *>(start), size, protections);
    if (ret < 0)
    {
        perror("mprotect failed");
    }
    return ret == 0;
}

class PosixPageFaultHandler : public PageFaultHandler
{
  public:
    PosixPageFaultHandler(PageFaultCallback callback) : PageFaultHandler(callback) {}
    ~PosixPageFaultHandler() override {}

    bool enable() override;
    bool disable() override;
    void handle(int sig, siginfo_t *info, void *unused);

  private:
    struct sigaction mDefaultBusAction  = {};
    struct sigaction mDefaultSegvAction = {};
};

PosixPageFaultHandler *gPosixPageFaultHandler = nullptr;
void SegfaultHandlerFunction(int sig, siginfo_t *info, void *unused)
{
    gPosixPageFaultHandler->handle(sig, info, unused);
}

void PosixPageFaultHandler::handle(int sig, siginfo_t *info, void *unused)
{
    bool found = false;
    if ((sig == SIGSEGV || sig == SIGBUS) &&
        (info->si_code == SEGV_ACCERR || info->si_code == SEGV_MAPERR))
    {
        found = mCallback(reinterpret_cast<uintptr_t>(info->si_addr)) ==
                PageFaultHandlerRangeType::InRange;
    }

    // Fall back to default signal handler
    if (!found)
    {
        if (sig == SIGSEGV)
        {
            mDefaultSegvAction.sa_sigaction(sig, info, unused);
        }
        else if (sig == SIGBUS)
        {
            mDefaultBusAction.sa_sigaction(sig, info, unused);
        }
        else
        {
            UNREACHABLE();
        }
    }
}

bool PosixPageFaultHandler::disable()
{
    return sigaction(SIGSEGV, &mDefaultSegvAction, nullptr) == 0 &&
           sigaction(SIGBUS, &mDefaultBusAction, nullptr) == 0;
}

bool PosixPageFaultHandler::enable()
{
    struct sigaction sigAction = {};
    sigAction.sa_flags         = SA_SIGINFO;
    sigAction.sa_sigaction     = &SegfaultHandlerFunction;
    sigemptyset(&sigAction.sa_mask);

    // Some POSIX implementations use SIGBUS for mprotect faults
    return sigaction(SIGSEGV, &sigAction, &mDefaultSegvAction) == 0 &&
           sigaction(SIGBUS, &sigAction, &mDefaultBusAction) == 0;
}
}  // namespace

// Set write protection
bool ProtectMemory(uintptr_t start, size_t size)
{
    return SetMemoryProtection(start, size, PROT_READ);
}

// Allow reading and writing
bool UnprotectMemory(uintptr_t start, size_t size)
{
    return SetMemoryProtection(start, size, PROT_READ | PROT_WRITE);
}

size_t GetPageSize()
{
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pageSize < 0)
    {
        perror("Could not get sysconf page size");
        return 0;
    }
    return static_cast<size_t>(pageSize);
}

PageFaultHandler *CreatePageFaultHandler(PageFaultCallback callback)
{
    gPosixPageFaultHandler = new PosixPageFaultHandler(callback);
    return gPosixPageFaultHandler;
}

uint64_t GetProcessMemoryUsageKB()
{
    FILE *file = fopen("/proc/self/status", "r");

    if (!file)
    {
        return 0;
    }

    const char *kSearchString           = "VmRSS:";
    constexpr size_t kMaxLineSize       = 100;
    std::array<char, kMaxLineSize> line = {};

    uint64_t kb = 0;

    while (fgets(line.data(), line.size(), file) != nullptr)
    {
        if (strncmp(line.data(), kSearchString, strlen(kSearchString)) == 0)
        {
            std::vector<std::string> strings;
            SplitStringAlongWhitespace(line.data(), &strings);

            sscanf(strings[1].c_str(), "%" SCNu64, &kb);
            break;
        }
    }
    fclose(file);

    return kb;
}
}  // namespace angle
