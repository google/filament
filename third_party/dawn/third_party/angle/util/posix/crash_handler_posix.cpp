//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// crash_handler_posix:
//    ANGLE's crash handling and stack walking code. Modified from Skia's:
//     https://github.com/google/skia/blob/master/tools/CrashHandler.cpp
//

#include "util/test_utils.h"

#include "common/FixedVector.h"
#include "common/angleutils.h"
#include "common/string_utils.h"
#include "common/system_utils.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

#if !defined(ANGLE_PLATFORM_ANDROID) && !defined(ANGLE_PLATFORM_FUCHSIA)
#    if defined(ANGLE_PLATFORM_APPLE)
// We only use local unwinding, so we can define this to select a faster implementation.
#        define UNW_LOCAL_ONLY
#        include <cxxabi.h>
#        include <libunwind.h>
#        include <signal.h>
#    elif defined(ANGLE_PLATFORM_POSIX)
// We'd use libunwind here too, but it's a pain to get installed for
// both 32 and 64 bit on bots.  Doesn't matter much: catchsegv is best anyway.
#        include <cxxabi.h>
#        include <dlfcn.h>
#        include <execinfo.h>
#        include <libgen.h>
#        include <link.h>
#        include <signal.h>
#        include <string.h>
#    endif  // defined(ANGLE_PLATFORM_APPLE)
#endif      // !defined(ANGLE_PLATFORM_ANDROID) && !defined(ANGLE_PLATFORM_FUCHSIA)

// This code snippet is coped from Chromium's base/posix/eintr_wrapper.h.
#if defined(NDEBUG)
#    define HANDLE_EINTR(x)                                         \
        ({                                                          \
            decltype(x) eintr_wrapper_result;                       \
            do                                                      \
            {                                                       \
                eintr_wrapper_result = (x);                         \
            } while (eintr_wrapper_result == -1 && errno == EINTR); \
            eintr_wrapper_result;                                   \
        })
#else
#    define HANDLE_EINTR(x)                                          \
        ({                                                           \
            int eintr_wrapper_counter = 0;                           \
            decltype(x) eintr_wrapper_result;                        \
            do                                                       \
            {                                                        \
                eintr_wrapper_result = (x);                          \
            } while (eintr_wrapper_result == -1 && errno == EINTR && \
                     eintr_wrapper_counter++ < 100);                 \
            eintr_wrapper_result;                                    \
        })
#endif  // NDEBUG

namespace angle
{
#if defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_FUCHSIA)

void PrintStackBacktrace()
{
    // No implementations yet.
}

void InitCrashHandler(CrashCallback *callback)
{
    // No implementations yet.
}

void TerminateCrashHandler()
{
    // No implementations yet.
}

#else
namespace
{
CrashCallback *gCrashHandlerCallback;
}  // namespace

#    if defined(ANGLE_PLATFORM_APPLE)

void PrintStackBacktrace()
{
    printf("Backtrace:\n");

    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0)
    {
        static const size_t kMax = 256;
        char mangled[kMax];
        unw_word_t offset;
        unw_get_proc_name(&cursor, mangled, kMax, &offset);

        int ok          = -1;
        char *demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &ok);
        printf("    %s (+0x%zx)\n", ok == 0 ? demangled : mangled, (size_t)offset);
        if (ok)
        {
            free(demangled);
        }
    }
    printf("\n");
}

static void Handler(int sig)
{
    printf("\nSignal %d:\n", sig);
    fflush(stdout);

    if (gCrashHandlerCallback)
    {
        (*gCrashHandlerCallback)();
    }

    PrintStackBacktrace();
    fflush(stdout);

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#    elif defined(ANGLE_PLATFORM_POSIX)

// Can control this at a higher level if required.
#        define ANGLE_HAS_ADDR2LINE

#        if defined(ANGLE_HAS_ADDR2LINE)
namespace
{
// The following code was adapted from Chromium's "stack_trace_posix.cc".
// Describes a region of mapped memory and the path of the file mapped.
struct MappedMemoryRegion
{
    enum Permission
    {
        READ    = 1 << 0,
        WRITE   = 1 << 1,
        EXECUTE = 1 << 2,
        PRIVATE = 1 << 3,  // If set, region is private, otherwise it is shared.
    };

    // The address range [start,end) of mapped memory.
    uintptr_t start;
    uintptr_t end;

    // Byte offset into |path| of the range mapped into memory.
    unsigned long long offset;

    // Image base, if this mapping corresponds to an ELF image.
    uintptr_t base;

    // Bitmask of read/write/execute/private/shared permissions.
    uint8_t permissions;

    // Name of the file mapped into memory.
    //
    // NOTE: path names aren't guaranteed to point at valid files. For example,
    // "[heap]" and "[stack]" are used to represent the location of the process'
    // heap and stack, respectively.
    std::string path;
};

using MemoryRegionArray = std::vector<MappedMemoryRegion>;

bool ReadProcMaps(std::string *proc_maps)
{
    // seq_file only writes out a page-sized amount on each call. Refer to header
    // file for details.
    const long kReadSize = sysconf(_SC_PAGESIZE);

    int fd(HANDLE_EINTR(open("/proc/self/maps", O_RDONLY)));
    if (fd == -1)
    {
        fprintf(stderr, "Couldn't open /proc/self/maps\n");
        return false;
    }
    proc_maps->clear();

    while (true)
    {
        // To avoid a copy, resize |proc_maps| so read() can write directly into it.
        // Compute |buffer| afterwards since resize() may reallocate.
        size_t pos = proc_maps->size();
        proc_maps->resize(pos + kReadSize);
        void *buffer = &(*proc_maps)[pos];

        ssize_t bytes_read = HANDLE_EINTR(read(fd, buffer, kReadSize));
        if (bytes_read < 0)
        {
            fprintf(stderr, "Couldn't read /proc/self/maps\n");
            proc_maps->clear();
            close(fd);
            return false;
        }

        // ... and don't forget to trim off excess bytes.
        proc_maps->resize(pos + bytes_read);

        if (bytes_read == 0)
            break;
    }

    close(fd);
    return true;
}

bool ParseProcMaps(const std::string &input, MemoryRegionArray *regions_out)
{
    ASSERT(regions_out);
    MemoryRegionArray regions;

    // This isn't async safe nor terribly efficient, but it doesn't need to be at
    // this point in time.
    std::vector<std::string> lines = SplitString(input, "\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        // Due to splitting on '\n' the last line should be empty.
        if (i == lines.size() - 1)
        {
            if (!lines[i].empty())
            {
                fprintf(stderr, "ParseProcMaps: Last line not empty");
                return false;
            }
            break;
        }

        MappedMemoryRegion region;
        const char *line    = lines[i].c_str();
        char permissions[5] = {'\0'};  // Ensure NUL-terminated string.
        uint8_t dev_major   = 0;
        uint8_t dev_minor   = 0;
        long inode          = 0;
        int path_index      = 0;

        // Sample format from man 5 proc:
        //
        // address           perms offset  dev   inode   pathname
        // 08048000-08056000 r-xp 00000000 03:0c 64593   /usr/sbin/gpm
        //
        // The final %n term captures the offset in the input string, which is used
        // to determine the path name. It *does not* increment the return value.
        // Refer to man 3 sscanf for details.
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %4c %llx %hhx:%hhx %ld %n", &region.start,
                   &region.end, permissions, &region.offset, &dev_major, &dev_minor, &inode,
                   &path_index) < 7)
        {
            fprintf(stderr, "ParseProcMaps: sscanf failed for line: %s\n", line);
            return false;
        }

        region.permissions = 0;

        if (permissions[0] == 'r')
            region.permissions |= MappedMemoryRegion::READ;
        else if (permissions[0] != '-')
            return false;

        if (permissions[1] == 'w')
            region.permissions |= MappedMemoryRegion::WRITE;
        else if (permissions[1] != '-')
            return false;

        if (permissions[2] == 'x')
            region.permissions |= MappedMemoryRegion::EXECUTE;
        else if (permissions[2] != '-')
            return false;

        if (permissions[3] == 'p')
            region.permissions |= MappedMemoryRegion::PRIVATE;
        else if (permissions[3] != 's' && permissions[3] != 'S')  // Shared memory.
            return false;

        // Pushing then assigning saves us a string copy.
        regions.push_back(region);
        regions.back().path.assign(line + path_index);
    }

    regions_out->swap(regions);
    return true;
}

// Set the base address for each memory region by reading ELF headers in
// process memory.
void SetBaseAddressesForMemoryRegions(MemoryRegionArray &regions)
{
    int mem_fd(HANDLE_EINTR(open("/proc/self/mem", O_RDONLY | O_CLOEXEC)));
    if (mem_fd == -1)
        return;

    auto safe_memcpy = [&mem_fd](void *dst, uintptr_t src, size_t size) {
        return HANDLE_EINTR(pread(mem_fd, dst, size, src)) == ssize_t(size);
    };

    uintptr_t cur_base = 0;
    for (MappedMemoryRegion &r : regions)
    {
        ElfW(Ehdr) ehdr;
        static_assert(SELFMAG <= sizeof(ElfW(Ehdr)), "SELFMAG too large");
        if ((r.permissions & MappedMemoryRegion::READ) &&
            safe_memcpy(&ehdr, r.start, sizeof(ElfW(Ehdr))) &&
            memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0)
        {
            switch (ehdr.e_type)
            {
                case ET_EXEC:
                    cur_base = 0;
                    break;
                case ET_DYN:
                    // Find the segment containing file offset 0. This will correspond
                    // to the ELF header that we just read. Normally this will have
                    // virtual address 0, but this is not guaranteed. We must subtract
                    // the virtual address from the address where the ELF header was
                    // mapped to get the base address.
                    //
                    // If we fail to find a segment for file offset 0, use the address
                    // of the ELF header as the base address.
                    cur_base = r.start;
                    for (unsigned i = 0; i != ehdr.e_phnum; ++i)
                    {
                        ElfW(Phdr) phdr;
                        if (safe_memcpy(&phdr, r.start + ehdr.e_phoff + i * sizeof(phdr),
                                        sizeof(phdr)) &&
                            phdr.p_type == PT_LOAD && phdr.p_offset == 0)
                        {
                            cur_base = r.start - phdr.p_vaddr;
                            break;
                        }
                    }
                    break;
                default:
                    // ET_REL or ET_CORE. These aren't directly executable, so they
                    // don't affect the base address.
                    break;
            }
        }

        r.base = cur_base;
    }

    close(mem_fd);
}

// Parses /proc/self/maps in order to compile a list of all object file names
// for the modules that are loaded in the current process.
// Returns true on success.
bool CacheMemoryRegions(MemoryRegionArray &regions)
{
    // Reads /proc/self/maps.
    std::string contents;
    if (!ReadProcMaps(&contents))
    {
        fprintf(stderr, "CacheMemoryRegions: Failed to read /proc/self/maps\n");
        return false;
    }

    // Parses /proc/self/maps.
    if (!ParseProcMaps(contents, &regions))
    {
        fprintf(stderr, "CacheMemoryRegions: Failed to parse the contents of /proc/self/maps\n");
        return false;
    }

    SetBaseAddressesForMemoryRegions(regions);
    return true;
}

constexpr size_t kAddr2LineMaxParameters = 50;
using Addr2LineCommandLine = angle::FixedVector<const char *, kAddr2LineMaxParameters>;

void CallAddr2Line(const Addr2LineCommandLine &commandLine)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        std::cerr << "Error: Failed to fork()" << std::endl;
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        // Ignore the status, since we aren't going to handle it anyway.
    }
    else
    {
        // Child process executes addr2line
        //
        // See comment in test_utils_posix.cpp::PosixProcess regarding const_cast.
        execvp(commandLine[0], const_cast<char *const *>(commandLine.data()));
        std::cerr << "Error: Child process returned from exevc()" << std::endl;
        _exit(EXIT_FAILURE);  // exec never returns
    }
}

constexpr size_t kMaxAddressLen = 1024;
using AddressBuffer             = angle::FixedVector<char, kMaxAddressLen>;

const char *ResolveAddress(const MemoryRegionArray &regions,
                           const std::string &resolvedModule,
                           const char *address,
                           AddressBuffer &buffer)
{
    size_t lastModuleSlash = resolvedModule.rfind('/');
    ASSERT(lastModuleSlash != std::string::npos);
    std::string baseModule = resolvedModule.substr(lastModuleSlash);

    for (const MappedMemoryRegion &region : regions)
    {
        size_t pathSlashPos = region.path.rfind('/');
        if (pathSlashPos != std::string::npos && region.path.substr(pathSlashPos) == baseModule)
        {
            uintptr_t scannedAddress;
            int scanReturn = sscanf(address, "%" SCNxPTR, &scannedAddress);
            ASSERT(scanReturn == 1);
            scannedAddress -= region.base;
            char printBuffer[255] = {};
            size_t scannedSize    = sprintf(printBuffer, "0x%" PRIXPTR, scannedAddress);
            size_t bufferSize     = buffer.size();
            buffer.resize(bufferSize + scannedSize + 1, 0);
            memcpy(&buffer[bufferSize], printBuffer, scannedSize);
            return &buffer[bufferSize];
        }
    }

    return address;
}
// This is only required when the current CWD does not match the initial CWD and could be replaced
// by storing the initial CWD state globally. It is only changed in vulkan_icd.cpp.
std::string RemoveOverlappingPath(const std::string &resolvedModule)
{
    // Build path from CWD in case CWD matches executable directory
    // but relative paths are from initial cwd.
    const Optional<std::string> &cwd = angle::GetCWD();
    if (!cwd.valid())
    {
        std::cerr << "Error getting CWD to print the backtrace." << std::endl;
        return resolvedModule;
    }
    else
    {
        std::string absolutePath = cwd.value();
        size_t lastPathSepLoc    = resolvedModule.find_last_of(GetPathSeparator());
        std::string relativePath = resolvedModule.substr(0, lastPathSepLoc);

        // Remove "." from the relativePath path
        // For example: ./out/LinuxDebug/angle_perftests
        size_t pos = relativePath.find('.');
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            relativePath.erase(pos, 1);
        }

        // Remove the overlapping relative path from the CWD so we can build the full
        // absolute path.
        // For example:
        // absolutePath = /home/timvp/code/angle/out/LinuxDebug
        // relativePath = /out/LinuxDebug
        pos = absolutePath.find(relativePath);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            absolutePath.erase(pos, relativePath.length());
        }
        return absolutePath + GetPathSeparator() + resolvedModule;
    }
}
}  // anonymous namespace
#        endif  // defined(ANGLE_HAS_ADDR2LINE)

void PrintStackBacktrace()
{
    printf("Backtrace:\n");

    void *stack[64];
    const int count = backtrace(stack, ArraySize(stack));
    char **symbols  = backtrace_symbols(stack, count);

#        if defined(ANGLE_HAS_ADDR2LINE)

    MemoryRegionArray regions;
    CacheMemoryRegions(regions);

    // Child process executes addr2line
    constexpr size_t kAddr2LineFixedParametersCount = 6;
    Addr2LineCommandLine commandLineArgs            = {
        "addr2line", "-s", "-p", "-f", "-C", "-e",
    };
    const char *currentModule = "";
    std::string resolvedModule;
    AddressBuffer addressBuffer;

    for (int i = 0; i < count; i++)
    {
        char *symbol = symbols[i];

        // symbol looks like the following:
        //
        //     path/to/module(+localAddress) [address]
        //
        // If module is not an absolute path, it needs to be resolved.

        char *module  = symbol;
        char *address = strchr(symbol, '[') + 1;

        *strchr(module, '(')  = 0;
        *strchr(address, ']') = 0;

        // If module is the same as last, continue batching addresses.  If commandLineArgs has
        // reached its capacity however, make the call to addr2line already.  Note that there should
        // be one entry left for the terminating nullptr at the end of the command line args.
        if (strcmp(module, currentModule) == 0 &&
            commandLineArgs.size() + 1 < commandLineArgs.max_size())
        {
            commandLineArgs.push_back(
                ResolveAddress(regions, resolvedModule, address, addressBuffer));
            continue;
        }

        // If there's a command batched, execute it before modifying currentModule (a pointer to
        // which is stored in the command line args).
        if (currentModule[0] != 0)
        {
            commandLineArgs.push_back(nullptr);
            CallAddr2Line(commandLineArgs);
            addressBuffer.clear();
        }

        // Reset the command line and remember this module as the current.
        resolvedModule = currentModule = module;
        commandLineArgs.resize(kAddr2LineFixedParametersCount);

        // First check if the a relative path simply resolved to an absolute one from cwd,
        // for abolute paths this resolves symlinks.
        char *realPath = realpath(resolvedModule.c_str(), NULL);
        if (realPath)
        {
            resolvedModule = std::string(realPath);
            free(realPath);
        }
        // We need an absolute path to get to the executable and all of the various shared objects,
        // but the caller may have used a relative path to launch the executable, so build one up if
        // we don't see a leading '/'.
        else if (resolvedModule.at(0) != GetPathSeparator())
        {
            // For some modules we receive a relative path from the build directory (executable
            // directory) instead of the execution directory (current directory). This happens
            // for libVkLayer_khronos_validation.so. If realpath fails to create an absolute
            // path, try constructing one from the build directory.
            // This will resolve paths like `angledata/../libVkLayer_khronos_validation.so` to
            // `/home/user/angle/out/Debug/libVkLayer_khronos_validation.so`
            std::string pathFromExecDir =
                GetExecutableDirectory() + GetPathSeparator() + resolvedModule;
            realPath = realpath(pathFromExecDir.c_str(), NULL);
            if (realPath)
            {
                resolvedModule = std::string(realPath);
                free(realPath);
            }
            else
            {
                // Try removing overlapping path as a last resort.
                // This will resolve `./out/Debug/angle_end2end_tests` to
                // `/home/user/angle/out/Debug/angle_end2end_tests` when CWD is
                // `/home/user/angle/out/Debug`, which is caused by ScopedVkLoaderEnvironment.
                // This is required for printing traces during vk::Renderer init.
                // Since we do not store the initial CWD globally we need to reconstruct here
                // by removing the overlapping path.
                std::string removeOverlappingPath = RemoveOverlappingPath(resolvedModule);
                realPath                          = realpath(removeOverlappingPath.c_str(), NULL);
                if (realPath)
                {
                    resolvedModule = std::string(realPath);
                    free(realPath);
                }
                else
                {
                    WARN() << "Could not resolve path for module with relative path "
                           << resolvedModule;
                }
            }
        }
        else
        {
            WARN() << "Could not resolve path for module with absolute path " << resolvedModule;
        }

        const char *resolvedAddress =
            ResolveAddress(regions, resolvedModule, address, addressBuffer);

        commandLineArgs.push_back(resolvedModule.c_str());
        commandLineArgs.push_back(resolvedAddress);
    }

    // Call addr2line for the last batch of addresses.
    if (currentModule[0] != 0)
    {
        commandLineArgs.push_back(nullptr);
        CallAddr2Line(commandLineArgs);
    }
#        else
    for (int i = 0; i < count; i++)
    {
        Dl_info info;
        if (dladdr(stack[i], &info) && info.dli_sname)
        {
            // Make sure this is large enough to hold the fully demangled names, otherwise we could
            // segault/hang here. For example, Vulkan validation layer errors can be deep enough
            // into the stack that very large symbol names are generated.
            char demangled[4096];
            size_t len = ArraySize(demangled);
            int ok;

            abi::__cxa_demangle(info.dli_sname, demangled, &len, &ok);
            if (ok == 0)
            {
                printf("    %s\n", demangled);
                continue;
            }
        }
        printf("    %s\n", symbols[i]);
    }
#        endif  // defined(ANGLE_HAS_ADDR2LINE)
}

static void Handler(int sig)
{
    printf("\nSignal %d [%s]:\n", sig, strsignal(sig));

    if (gCrashHandlerCallback)
    {
        (*gCrashHandlerCallback)();
    }

    PrintStackBacktrace();

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#    endif  // defined(ANGLE_PLATFORM_APPLE)

static constexpr int kSignals[] = {
    SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP,
};

void InitCrashHandler(CrashCallback *callback)
{
    gCrashHandlerCallback = callback;
    for (int sig : kSignals)
    {
        // Register our signal handler unless something's already done so (e.g. catchsegv).
        void (*prev)(int) = signal(sig, Handler);
        if (prev != SIG_DFL)
        {
            signal(sig, prev);
        }
    }
}

void TerminateCrashHandler()
{
    gCrashHandlerCallback = nullptr;
    for (int sig : kSignals)
    {
        void (*prev)(int) = signal(sig, SIG_DFL);
        if (prev != Handler && prev != SIG_DFL)
        {
            signal(sig, prev);
        }
    }
}

#endif  // defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_FUCHSIA)

}  // namespace angle
