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

#pragma once

#include <filesystem>

#include "test_defines.h"

#include "functions.h"

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#include <strsafe.h>
#elif TESTING_COMMON_UNIX_PLATFORMS
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

struct LibraryWrapper {
    explicit LibraryWrapper() noexcept;
    explicit LibraryWrapper(std::filesystem::path const& lib_path) noexcept;
    ~LibraryWrapper() noexcept;
    LibraryWrapper(LibraryWrapper const& wrapper) = delete;
    LibraryWrapper& operator=(LibraryWrapper const& wrapper) = delete;
    LibraryWrapper(LibraryWrapper&& wrapper) noexcept;
    LibraryWrapper& operator=(LibraryWrapper&& wrapper) noexcept;
    FromVoidStarFunc get_symbol(const char* symbol_name) const;

    std::filesystem::path const& get_path() const { return lib_path; }
    explicit operator bool() const noexcept { return lib_handle != nullptr; }

   private:
    void close_library() noexcept;

#if defined(_WIN32)
    HMODULE lib_handle = nullptr;
#elif TESTING_COMMON_UNIX_PLATFORMS
    void* lib_handle = nullptr;
#else
#error "Unhandled platform in dynamic_library_wrapper.h!"
#endif
    std::filesystem::path lib_path;
};
