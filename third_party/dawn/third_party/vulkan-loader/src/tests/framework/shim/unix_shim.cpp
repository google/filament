/*
 * Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2022 Valve Corporation
 * Copyright (c) 2021-2022 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "shim.h"

#include <algorithm>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

PlatformShim platform_shim;
extern "C" {
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__) || defined(__QNX__)
PlatformShim* get_platform_shim(std::vector<fs::FolderManager>* folders) {
    platform_shim = PlatformShim(folders);
    return &platform_shim;
}
#elif defined(__APPLE__)
FRAMEWORK_EXPORT PlatformShim* get_platform_shim(std::vector<fs::FolderManager>* folders) {
    platform_shim = PlatformShim(folders);
    return &platform_shim;
}
#endif

// Necessary for MacOS function shimming
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__) || defined(__QNX__)
#define OPENDIR_FUNC_NAME opendir
#define READDIR_FUNC_NAME readdir
#define CLOSEDIR_FUNC_NAME closedir
#define ACCESS_FUNC_NAME access
#define FOPEN_FUNC_NAME fopen
#define DLOPEN_FUNC_NAME dlopen
#define GETEUID_FUNC_NAME geteuid
#define GETEGID_FUNC_NAME getegid
#if defined(HAVE_SECURE_GETENV)
#define SECURE_GETENV_FUNC_NAME secure_getenv
#endif
#if defined(HAVE___SECURE_GETENV)
#define __SECURE_GETENV_FUNC_NAME __secure_getenv
#endif
#define PRINTF_FUNC_NAME printf
#define FPUTS_FUNC_NAME fputs
#define FPUTC_FUNC_NAME fputc
#elif defined(__APPLE__)
#define OPENDIR_FUNC_NAME my_opendir
#define READDIR_FUNC_NAME my_readdir
#define CLOSEDIR_FUNC_NAME my_closedir
#define ACCESS_FUNC_NAME my_access
#define FOPEN_FUNC_NAME my_fopen
#define DLOPEN_FUNC_NAME my_dlopen
#define GETEUID_FUNC_NAME my_geteuid
#define GETEGID_FUNC_NAME my_getegid
#if !defined(TARGET_OS_IPHONE)
#if defined(HAVE_SECURE_GETENV)
#define SECURE_GETENV_FUNC_NAME my_secure_getenv
#endif
#if defined(HAVE___SECURE_GETENV)
#define __SECURE_GETENV_FUNC_NAME my__secure_getenv
#endif
#endif
#define FPUTS_FUNC_NAME my_fputs
#define FPUTC_FUNC_NAME my_fputc
#endif

using PFN_OPENDIR = DIR* (*)(const char* path_name);
using PFN_READDIR = struct dirent* (*)(DIR* dir_stream);
using PFN_CLOSEDIR = int (*)(DIR* dir_stream);
using PFN_ACCESS = int (*)(const char* pathname, int mode);
using PFN_FOPEN = FILE* (*)(const char* filename, const char* mode);
using PFN_DLOPEN = void* (*)(const char* in_filename, int flags);
using PFN_GETEUID = uid_t (*)(void);
using PFN_GETEGID = gid_t (*)(void);
#if defined(HAVE_SECURE_GETENV) || defined(HAVE___SECURE_GETENV)
using PFN_SEC_GETENV = char* (*)(const char* name);
#endif
using PFN_FPUTS = int (*)(const char* str, FILE* stream);
using PFN_FPUTC = int (*)(int c, FILE* stream);

#if defined(__APPLE__)
#define real_opendir opendir
#define real_readdir readdir
#define real_closedir closedir
#define real_access access
#define real_fopen fopen
#define real_dlopen dlopen
#define real_geteuid geteuid
#define real_getegid getegid
#if defined(HAVE_SECURE_GETENV)
#define real_secure_getenv secure_getenv
#endif
#if defined(HAVE___SECURE_GETENV)
#define real__secure_getenv __secure_getenv
#endif
#define real_fputs fputs
#define real_fputc fputc
#else
PFN_OPENDIR real_opendir = nullptr;
PFN_READDIR real_readdir = nullptr;
PFN_CLOSEDIR real_closedir = nullptr;
PFN_ACCESS real_access = nullptr;
PFN_FOPEN real_fopen = nullptr;
PFN_DLOPEN real_dlopen = nullptr;
PFN_GETEUID real_geteuid = nullptr;
PFN_GETEGID real_getegid = nullptr;
#if defined(HAVE_SECURE_GETENV)
PFN_SEC_GETENV real_secure_getenv = nullptr;
#endif
#if defined(HAVE___SECURE_GETENV)
PFN_SEC_GETENV real__secure_getenv = nullptr;
#endif
PFN_FPUTS real_fputs = nullptr;
PFN_FPUTC real_fputc = nullptr;
#endif

FRAMEWORK_EXPORT DIR* OPENDIR_FUNC_NAME(const char* path_name) {
#if !defined(__APPLE__)
    if (!real_opendir) real_opendir = (PFN_OPENDIR)dlsym(RTLD_NEXT, "opendir");
#endif
    if (platform_shim.is_during_destruction) {
        return real_opendir(path_name);
    }
    DIR* dir;
    if (platform_shim.is_fake_path(path_name)) {
        auto real_path_name = platform_shim.get_real_path_from_fake_path(std::filesystem::path(path_name));
        dir = real_opendir(real_path_name.c_str());
        platform_shim.dir_entries.push_back(DirEntry{dir, std::string(path_name), {}, 0, true});
    } else if (platform_shim.is_known_path(path_name)) {
        dir = real_opendir(path_name);
        platform_shim.dir_entries.push_back(DirEntry{dir, std::string(path_name), {}, 0, false});
    } else {
        dir = real_opendir(path_name);
    }

    return dir;
}

FRAMEWORK_EXPORT struct dirent* READDIR_FUNC_NAME(DIR* dir_stream) {
#if !defined(__APPLE__)
    if (!real_readdir) {
        if (sizeof(void*) == 8) {
            real_readdir = (PFN_READDIR)dlsym(RTLD_NEXT, "readdir");
        } else {
            // Necessary to specify the 64 bit readdir version since that is what is linked in when using _FILE_OFFSET_BITS
            real_readdir = (PFN_READDIR)dlsym(RTLD_NEXT, "readdir64");
        }
    }

#endif
    if (platform_shim.is_during_destruction) {
        return real_readdir(dir_stream);
    }
    auto it = std::find_if(platform_shim.dir_entries.begin(), platform_shim.dir_entries.end(),
                           [dir_stream](DirEntry const& entry) { return entry.directory == dir_stream; });

    if (it == platform_shim.dir_entries.end()) {
        return real_readdir(dir_stream);
    }
    // Folder was found but this is the first file to be read from it
    if (it->current_index == 0) {
        std::vector<struct dirent*> folder_contents;
        std::vector<std::string> dirent_filenames;
        while (true) {
            errno = 0;
            struct dirent* dir_entry = real_readdir(dir_stream);
            if (errno != 0) {
                std::cerr << "Readdir failed: errno has value of " << std::to_string(errno) << "\n";
            }
            if (NULL == dir_entry) {
                break;
            }
            // skip . and .. entries
            if ((dir_entry->d_name[0] == '.' && dir_entry->d_name[1] == '.' && dir_entry->d_name[2] == '\0') ||
                (dir_entry->d_name[0] == '.' && dir_entry->d_name[1] == '\0')) {
                continue;
            }
            folder_contents.push_back(dir_entry);
            dirent_filenames.push_back(&dir_entry->d_name[0]);
        }
        auto real_path = it->folder_path;
        if (it->is_fake_path) {
            real_path = platform_shim.redirection_map.at(it->folder_path);
        }
        auto filenames = get_folder_contents(platform_shim.folders, real_path);

        // Add the dirent structures in the order they appear in the FolderManager
        // Ignore anything which wasn't in the FolderManager
        for (auto const& file : filenames) {
            for (size_t i = 0; i < dirent_filenames.size(); i++) {
                if (file == dirent_filenames.at(i)) {
                    it->contents.push_back(folder_contents.at(i));
                    break;
                }
            }
        }
    }
    if (it->current_index >= it->contents.size()) return nullptr;
    return it->contents.at(it->current_index++);
}

FRAMEWORK_EXPORT int CLOSEDIR_FUNC_NAME(DIR* dir_stream) {
#if !defined(__APPLE__)
    if (!real_closedir) real_closedir = (PFN_CLOSEDIR)dlsym(RTLD_NEXT, "closedir");
#endif
    if (platform_shim.is_during_destruction) {
        return real_closedir(dir_stream);
    }
    auto it = std::find_if(platform_shim.dir_entries.begin(), platform_shim.dir_entries.end(),
                           [dir_stream](DirEntry const& entry) { return entry.directory == dir_stream; });

    if (it != platform_shim.dir_entries.end()) {
        platform_shim.dir_entries.erase(it);
    }

    return real_closedir(dir_stream);
}

FRAMEWORK_EXPORT int ACCESS_FUNC_NAME(const char* in_pathname, int mode) {
#if !defined(__APPLE__)
    if (!real_access) real_access = (PFN_ACCESS)dlsym(RTLD_NEXT, "access");
#endif
    std::filesystem::path path{in_pathname};
    if (!path.has_parent_path()) {
        return real_access(in_pathname, mode);
    }

    if (platform_shim.is_fake_path(path.parent_path())) {
        std::filesystem::path real_path = platform_shim.get_real_path_from_fake_path(path.parent_path());
        real_path /= path.filename();
        return real_access(real_path.c_str(), mode);
    }
    return real_access(in_pathname, mode);
}

FRAMEWORK_EXPORT FILE* FOPEN_FUNC_NAME(const char* in_filename, const char* mode) {
#if !defined(__APPLE__)
    if (!real_fopen) real_fopen = (PFN_FOPEN)dlsym(RTLD_NEXT, "fopen");
#endif
    std::filesystem::path path{in_filename};
    if (!path.has_parent_path()) {
        return real_fopen(in_filename, mode);
    }

    FILE* f_ptr;
    if (platform_shim.is_fake_path(path.parent_path())) {
        auto real_path = platform_shim.get_real_path_from_fake_path(path.parent_path()) / path.filename();
        f_ptr = real_fopen(real_path.c_str(), mode);
    } else {
        f_ptr = real_fopen(in_filename, mode);
    }

    return f_ptr;
}

FRAMEWORK_EXPORT void* DLOPEN_FUNC_NAME(const char* in_filename, int flags) {
#if !defined(__APPLE__)
    if (!real_dlopen) real_dlopen = (PFN_DLOPEN)dlsym(RTLD_NEXT, "dlopen");
#endif

    if (platform_shim.is_dlopen_redirect_name(in_filename)) {
        return real_dlopen(platform_shim.dlopen_redirection_map[in_filename].c_str(), flags);
    }
    return real_dlopen(in_filename, flags);
}

FRAMEWORK_EXPORT uid_t GETEUID_FUNC_NAME(void) {
#if !defined(__APPLE__)
    if (!real_geteuid) real_geteuid = (PFN_GETEUID)dlsym(RTLD_NEXT, "geteuid");
#endif
    if (platform_shim.use_fake_elevation) {
        // Root on linux is 0, so force pretending like we're root
        return 0;
    } else {
        return real_geteuid();
    }
}

FRAMEWORK_EXPORT gid_t GETEGID_FUNC_NAME(void) {
#if !defined(__APPLE__)
    if (!real_getegid) real_getegid = (PFN_GETEGID)dlsym(RTLD_NEXT, "getegid");
#endif
    if (platform_shim.use_fake_elevation) {
        // Root on linux is 0, so force pretending like we're root
        return 0;
    } else {
        return real_getegid();
    }
}

#if !defined(TARGET_OS_IPHONE)
#if defined(HAVE_SECURE_GETENV)
FRAMEWORK_EXPORT char* SECURE_GETENV_FUNC_NAME(const char* name) {
#if !defined(__APPLE__)
    if (!real_secure_getenv) real_secure_getenv = (PFN_SEC_GETENV)dlsym(RTLD_NEXT, "secure_getenv");
#endif
    if (platform_shim.use_fake_elevation) {
        return NULL;
    } else {
        return real_secure_getenv(name);
    }
}
#endif
#if defined(HAVE___SECURE_GETENV)
FRAMEWORK_EXPORT char* __SECURE_GETENV_FUNC_NAME(const char* name) {
#if !defined(__APPLE__)
    if (!real__secure_getenv) real__secure_getenv = (PFN_SEC_GETENV)dlsym(RTLD_NEXT, "__secure_getenv");
#endif

    if (platform_shim.use_fake_elevation) {
        return NULL;
    } else {
        return real__secure_getenv(name);
    }
}
#endif
#endif

FRAMEWORK_EXPORT int FPUTS_FUNC_NAME(const char* str, FILE* stream) {
#if !defined(__APPLE__)
    if (!real_fputs) real_fputs = (PFN_FPUTS)dlsym(RTLD_NEXT, "fputs");
#endif
    if (stream == stderr) {
        platform_shim.fputs_stderr_log += str;
    }
    return real_fputs(str, stream);
}

FRAMEWORK_EXPORT int FPUTC_FUNC_NAME(int ch, FILE* stream) {
#if !defined(__APPLE__)
    if (!real_fputc) real_fputc = (PFN_FPUTC)dlsym(RTLD_NEXT, "fputc");
#endif
    if (stream == stderr) {
        platform_shim.fputs_stderr_log += ch;
    }
    return real_fputc(ch, stream);
}

#if defined(__APPLE__)
FRAMEWORK_EXPORT CFBundleRef my_CFBundleGetMainBundle() {
    static CFBundleRef global_bundle{};
    return reinterpret_cast<CFBundleRef>(&global_bundle);
}
FRAMEWORK_EXPORT CFURLRef my_CFBundleCopyResourcesDirectoryURL([[maybe_unused]] CFBundleRef bundle) {
    static CFURLRef global_url{};
    return reinterpret_cast<CFURLRef>(&global_url);
}
FRAMEWORK_EXPORT Boolean my_CFURLGetFileSystemRepresentation([[maybe_unused]] CFURLRef url,
                                                             [[maybe_unused]] Boolean resolveAgainstBase, UInt8* buffer,
                                                             CFIndex maxBufLen) {
    if (!platform_shim.bundle_contents.empty()) {
        CFIndex copy_len = (CFIndex)platform_shim.bundle_contents.size();
        if (copy_len > maxBufLen) {
            copy_len = maxBufLen;
        }
        strncpy(reinterpret_cast<char*>(buffer), platform_shim.bundle_contents.c_str(), copy_len);
        return TRUE;
    }
    return FALSE;
}
#endif

/* Shiming functions on apple is limited by the linker prefering to not use functions in the
 * executable in loaded dylibs. By adding an interposer, we redirect the linker to use our
 * version of the function over the real one, thus shimming the system function.
 */
#if defined(__APPLE__)
#define MACOS_ATTRIB __attribute__((section("__DATA,__interpose")))
#define VOIDP_CAST(_func) reinterpret_cast<const void*>(&_func)

struct Interposer {
    const void* shim_function;
    const void* underlying_function;
};

__attribute__((used)) static Interposer _interpose_opendir MACOS_ATTRIB = {VOIDP_CAST(my_opendir), VOIDP_CAST(opendir)};
__attribute__((used)) static Interposer _interpose_readdir MACOS_ATTRIB = {VOIDP_CAST(my_readdir), VOIDP_CAST(readdir)};
__attribute__((used)) static Interposer _interpose_closedir MACOS_ATTRIB = {VOIDP_CAST(my_closedir), VOIDP_CAST(closedir)};
__attribute__((used)) static Interposer _interpose_access MACOS_ATTRIB = {VOIDP_CAST(my_access), VOIDP_CAST(access)};
__attribute__((used)) static Interposer _interpose_fopen MACOS_ATTRIB = {VOIDP_CAST(my_fopen), VOIDP_CAST(fopen)};
__attribute__((used)) static Interposer _interpose_dlopen MACOS_ATTRIB = {VOIDP_CAST(my_dlopen), VOIDP_CAST(dlopen)};
__attribute__((used)) static Interposer _interpose_euid MACOS_ATTRIB = {VOIDP_CAST(my_geteuid), VOIDP_CAST(geteuid)};
__attribute__((used)) static Interposer _interpose_egid MACOS_ATTRIB = {VOIDP_CAST(my_getegid), VOIDP_CAST(getegid)};
#if !defined(TARGET_OS_IPHONE)
#if defined(HAVE_SECURE_GETENV)
__attribute__((used)) static Interposer _interpose_secure_getenv MACOS_ATTRIB = {VOIDP_CAST(my_secure_getenv),
                                                                                 VOIDP_CAST(secure_getenv)};
#endif
#if defined(HAVE___SECURE_GETENV)
__attribute__((used)) static Interposer _interpose__secure_getenv MACOS_ATTRIB = {VOIDP_CAST(my__secure_getenv),
                                                                                  VOIDP_CAST(__secure_getenv)};
#endif
#endif
__attribute__((used)) static Interposer _interpose_fputs MACOS_ATTRIB = {VOIDP_CAST(my_fputs), VOIDP_CAST(fputs)};
__attribute__((used)) static Interposer _interpose_fputc MACOS_ATTRIB = {VOIDP_CAST(my_fputc), VOIDP_CAST(fputc)};
__attribute__((used)) static Interposer _interpose_CFBundleGetMainBundle MACOS_ATTRIB = {VOIDP_CAST(my_CFBundleGetMainBundle),
                                                                                         VOIDP_CAST(CFBundleGetMainBundle)};
__attribute__((used)) static Interposer _interpose_CFBundleCopyResourcesDirectoryURL MACOS_ATTRIB = {
    VOIDP_CAST(my_CFBundleCopyResourcesDirectoryURL), VOIDP_CAST(CFBundleCopyResourcesDirectoryURL)};
__attribute__((used)) static Interposer _interpose_CFURLGetFileSystemRepresentation MACOS_ATTRIB = {
    VOIDP_CAST(my_CFURLGetFileSystemRepresentation), VOIDP_CAST(CFURLGetFileSystemRepresentation)};

#endif
}  // extern "C"
