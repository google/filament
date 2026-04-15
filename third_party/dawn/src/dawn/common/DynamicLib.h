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

#ifndef SRC_DAWN_COMMON_DYNAMICLIB_H_
#define SRC_DAWN_COMMON_DYNAMICLIB_H_

#include <span>
#include <string>
#include <type_traits>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "partition_alloc/pointers/raw_ptr.h"
#elif DAWN_PLATFORM_IS(POSIX)
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#else
#error "Unsupported platform for DynamicLib"
#endif

namespace dawn {

class DynamicLib {
  public:
    DynamicLib() = default;
    ~DynamicLib();

    DynamicLib(const DynamicLib&) = delete;
    DynamicLib& operator=(const DynamicLib&) = delete;

    DynamicLib(DynamicLib&& other);
    DynamicLib& operator=(DynamicLib&& other);

    bool Valid() const;

#if DAWN_PLATFORM_IS(WINDOWS) && !DAWN_PLATFORM_IS(WINUWP)
    bool OpenSystemLibrary(std::wstring_view filename, std::string* error = nullptr);
#endif
    bool Open(const std::string& filename, std::string* error = nullptr);
    bool Open(const std::string& filename,
              std::span<const std::string> searchPaths,
              std::string* error = nullptr);
    bool OpenLoaded(const std::string& filename, std::string* error = nullptr);
    void Close();

    void* GetProc(const std::string& procName, std::string* error = nullptr) const;

    template <typename T>
        requires std::is_function_v<T>
    bool GetProc(T** proc, const std::string& procName, std::string* error = nullptr) const {
        DAWN_ASSERT(proc != nullptr);

        *proc = reinterpret_cast<T*>(GetProc(procName, error));
        return *proc != nullptr;
    }

  private:
#if DAWN_PLATFORM_IS(WINDOWS)
    // This is an HMODULE (aka void*). It should point to real memory, so we can use raw_ptr:
    // > A handle to a module. This is the base address of the module in memory.
    raw_ptr<void> mHandle = nullptr;
#elif DAWN_PLATFORM_IS(POSIX)
    // On POSIX we use `dlopen`, which returns a "handle" which may not be a real pointer:
    // > The value of this symbol table handle should not be interpreted in any way by the caller.
    RAW_PTR_EXCLUSION void* mHandle = nullptr;
#else
#error "Unsupported platform for DynamicLib"
#endif

    bool mNeedsClose = false;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_DYNAMICLIB_H_
