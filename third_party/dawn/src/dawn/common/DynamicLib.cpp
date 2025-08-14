// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/common/DynamicLib.h"

#include <span>
#include <utility>

#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#if DAWN_PLATFORM_IS(WINUWP)
#include "dawn/common/WindowsUtils.h"
#endif
#elif DAWN_PLATFORM_IS(POSIX)
#include <dlfcn.h>
#else
#error "Unsupported platform for DynamicLib"
#endif

namespace dawn {

DynamicLib::~DynamicLib() {
    Close();
}

DynamicLib::DynamicLib(DynamicLib&& other) {
    std::swap(mHandle, other.mHandle);
}

DynamicLib& DynamicLib::operator=(DynamicLib&& other) {
    std::swap(mHandle, other.mHandle);
    return *this;
}

bool DynamicLib::Valid() const {
    return mHandle != nullptr;
}

#if DAWN_PLATFORM_IS(WINDOWS) && !DAWN_PLATFORM_IS(WINUWP)
bool DynamicLib::OpenSystemLibrary(std::wstring_view filename, std::string* error) {
    // Force LOAD_LIBRARY_SEARCH_SYSTEM32 for system libraries to avoid DLL search path
    // attacks.
    mHandle = ::LoadLibraryExW(filename.data(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (mHandle == nullptr && error != nullptr) {
        *error = "Windows Error: " + std::to_string(GetLastError());
    }
    return mHandle != nullptr;
}
#endif

bool DynamicLib::Open(const std::string& filename, std::string* error) {
#if DAWN_PLATFORM_IS(WINDOWS)
#if DAWN_PLATFORM_IS(WINUWP)
    mHandle = LoadPackagedLibrary(UTF8ToWStr(filename.c_str()).c_str(), 0);
#else
#if defined(DAWN_FORCE_SYSTEM_COMPONENT_LOAD)
    const DWORD loadLibraryFlags = LOAD_LIBRARY_SEARCH_SYSTEM32;
#else
    // Use SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS to avoid DLL search path attacks.
    const DWORD loadLibraryFlags =
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
#endif
    mHandle = LoadLibraryExA(filename.c_str(), nullptr, loadLibraryFlags);
#endif
    if (mHandle == nullptr && error != nullptr) {
        *error =
            "DynamicLib.Open: " + filename + " Windows Error: " + std::to_string(GetLastError());
    }
#elif DAWN_PLATFORM_IS(POSIX)
    mHandle = dlopen(filename.c_str(), RTLD_NOW);

    if (mHandle == nullptr && error != nullptr) {
        *error = dlerror();
    }
#else
#error "Unsupported platform for DynamicLib"
#endif

    return mHandle != nullptr;
}

bool DynamicLib::Open(const std::string& filename,
                      std::span<const std::string> searchPaths,
                      std::string* error) {
    for (const std::string& path : searchPaths) {
        const std::string fullPath = path + filename;
        if (Open(fullPath, error)) {
            return true;
        }
    }
    return false;
}

void DynamicLib::Close() {
    if (mHandle == nullptr) {
        return;
    }

#if DAWN_PLATFORM_IS(WINDOWS)
#if !DAWN_ASAN_ENABLED()
    // Freeing and reloading a DLL on Windows causes ASAN to detect ODR violations.
    // https://github.com/google/sanitizers/issues/89
    // In ASAN builds, we have to leak the DLL instead in case it gets loaded again later.
    FreeLibrary(static_cast<HMODULE>(mHandle));
#endif
#elif DAWN_PLATFORM_IS(POSIX)
    dlclose(mHandle);
#else
#error "Unsupported platform for DynamicLib"
#endif

    mHandle = nullptr;
}

void* DynamicLib::GetProc(const std::string& procName, std::string* error) const {
    void* proc = nullptr;

#if DAWN_PLATFORM_IS(WINDOWS)
    proc = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(mHandle), procName.c_str()));

    if (proc == nullptr && error != nullptr) {
        *error = "Windows Error: " + std::to_string(GetLastError());
    }
#elif DAWN_PLATFORM_IS(POSIX)
    proc = reinterpret_cast<void*>(dlsym(mHandle, procName.c_str()));

    if (proc == nullptr && error != nullptr) {
        *error = dlerror();
    }
#else
#error "Unsupported platform for DynamicLib"
#endif

    return proc;
}

}  // namespace dawn
