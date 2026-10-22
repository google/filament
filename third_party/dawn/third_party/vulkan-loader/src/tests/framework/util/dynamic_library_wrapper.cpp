/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#include "dynamic_library_wrapper.h"

#include "test_defines.h"
#include "env_var_wrapper.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <iostream>

namespace detail {

#if defined(_WIN32)
typedef HMODULE test_platform_dl_handle;

std::string error_code_to_string(DWORD error_code) {
    LPSTR lpMsgBuf{};
    size_t MsgBufSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                      nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, nullptr);
    std::string code_str(lpMsgBuf, MsgBufSize);
    LocalFree(lpMsgBuf);
    return code_str;
}
std::wstring wide_error_code_to_string(DWORD error_code) {
    LPWSTR lpwMsgBuf{};
    size_t MsgBufSize = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpwMsgBuf, 0, nullptr);
    std::wstring code_str(lpwMsgBuf, MsgBufSize);
    LocalFree(lpwMsgBuf);
    return code_str;
}
#elif TESTING_COMMON_UNIX_PLATFORMS
#include <dlfcn.h>

typedef void* test_platform_dl_handle;
#endif
}  // namespace detail

LibraryWrapper::LibraryWrapper() noexcept {}
LibraryWrapper::LibraryWrapper(std::filesystem::path const& lib_path) noexcept : lib_path(lib_path.lexically_normal()) {
#if defined(_WIN32)
    auto wide_path = lib_path.native();  // Get a std::wstring first
    // Try loading the library the original way first.
    lib_handle = LoadLibraryW(wide_path.c_str());
    DWORD last_error = GetLastError();
    // If that failed because of path limitations, prepend a special sequence to opt into longer path support
    if (lib_handle == nullptr && last_error == ERROR_FILENAME_EXCED_RANGE) {
        // "\\?\" is a special prefix for filenames that tells winapi to ignore path limits, so we can workaround path limitations
        // without relying on system level changes.
        wide_path = L"\\\\?\\" + wide_path;
        last_error = 0;
        lib_handle = LoadLibraryW(wide_path.c_str());
        last_error = GetLastError();
    }
    // If that failed, then try loading it with broader search folders.
    if (lib_handle == nullptr && last_error == ERROR_MOD_NOT_FOUND) {
        last_error = 0;
        lib_handle =
            LoadLibraryExW(wide_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
        last_error = GetLastError();
    }
    // If neither of the above worked, print an error message and abort
    if (lib_handle == nullptr) {
        auto error_msg = detail::wide_error_code_to_string(last_error);
        std::wcerr << L"Unable to open library: " << wide_path << L" due to: " << error_msg << L"\n";
        abort();
    }
#elif TESTING_COMMON_UNIX_PLATFORMS
    lib_handle = dlopen(lib_path.string().c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (lib_handle == nullptr) {
        std::wcerr << "Unable to open library: " << lib_path << " due to: " << dlerror() << "\n";
        abort();
    }
#else
#error "Unhandled platform in dynamic_library_wrapper.cpp!"
#endif
}

LibraryWrapper::~LibraryWrapper() noexcept { close_library(); }

LibraryWrapper::LibraryWrapper(LibraryWrapper&& wrapper) noexcept : lib_handle(wrapper.lib_handle), lib_path(wrapper.lib_path) {
    wrapper.lib_handle = nullptr;
}
LibraryWrapper& LibraryWrapper::operator=(LibraryWrapper&& wrapper) noexcept {
    if (this != &wrapper) {
        lib_handle = wrapper.lib_handle;
        lib_path = wrapper.lib_path;
        wrapper.lib_handle = nullptr;
    }
    return *this;
}

void LibraryWrapper::close_library() noexcept {
    auto loader_disable_dynamic_library_unloading_env_var = get_env_var("VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING", false);
    if (loader_disable_dynamic_library_unloading_env_var.empty() || loader_disable_dynamic_library_unloading_env_var != "1") {
        if (lib_handle != nullptr) {
#if defined(_WIN32)
            FreeLibrary(lib_handle);
#elif TESTING_COMMON_UNIX_PLATFORMS
            dlclose(lib_handle);
#else
#error "Unhandled platform in dynamic_library_wrapper.cpp!"
#endif
            lib_handle = nullptr;
        }
    }
}

FromVoidStarFunc LibraryWrapper::get_symbol(const char* symbol_name) const {
    if (symbol_name == nullptr) {
        std::cerr << "LibraryWrapper::get_symbol called with a null symbol_name!\n";
        abort();
    }
    if (lib_handle == nullptr) {
        std::cerr << "LibraryWrapper::get_symbol called when lib_handle is nullptr!\n";
        abort();
    }

#if defined(_WIN32)
    void* symbol = reinterpret_cast<void*>(GetProcAddress(lib_handle, symbol_name));
    if (symbol == nullptr) {
        auto last_error = detail::error_code_to_string(GetLastError());
        std::cerr << "Unable to get symbol " << symbol_name << " in library " << lib_path.string() << " with to error code "
                  << last_error << "\n";
        abort();
    }
#elif TESTING_COMMON_UNIX_PLATFORMS
    void* symbol = dlsym(lib_handle, symbol_name);
    if (symbol == nullptr) {
        std::cerr << "Unable to get symbol " << symbol_name << " in library " << lib_path.string() << " due to " << dlerror()
                  << "\n";
        abort();
    }
#else
#error "Unhandled platform in dynamic_library_wrapper.cpp!"
#endif
    return FromVoidStarFunc(symbol);
}
