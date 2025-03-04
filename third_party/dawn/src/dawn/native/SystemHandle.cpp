// Copyright 2023 The Dawn & Tint Authors
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

#include "src/dawn/native/SystemHandle.h"

#include <utility>

#include "dawn/common/Log.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#elif DAWN_PLATFORM_IS(FUCHSIA)
#include <zircon/syscalls.h>
#elif DAWN_PLATFORM_IS(POSIX)
#include <unistd.h>
#endif

namespace dawn::native {

namespace {

#if DAWN_PLATFORM_IS(WINDOWS)

constexpr inline HANDLE kInvalidHandle = nullptr;

inline bool IsHandleValid(HANDLE handle) {
    return handle != nullptr;
}

inline ResultOrError<HANDLE> DuplicateHandle(HANDLE handle) {
    HANDLE currentProcess = ::GetCurrentProcess();
    HANDLE outHandle;
    DAWN_INTERNAL_ERROR_IF(!::DuplicateHandle(currentProcess, handle, currentProcess, &outHandle, 0,
                                              FALSE, DUPLICATE_SAME_ACCESS),
                           "DuplicateHandle failed");
    return outHandle;
}

inline MaybeError CloseHandle(HANDLE handle) {
    DAWN_INTERNAL_ERROR_IF(!::CloseHandle(handle), "CloseHandle failed");
    return {};
}

#elif DAWN_PLATFORM_IS(FUCHSIA)

constexpr inline zx_handle_t kInvalidHandle = 0;

inline bool IsHandleValid(zx_handle_t handle) {
    return handle > 0;
}

inline ResultOrError<zx_handle_t> DuplicateHandle(zx_handle_t handle) {
    zx_handle_t outHandle = ZX_HANDLE_INVALID;
    DAWN_INTERNAL_ERROR_IF(zx_handle_duplicate(handle, ZX_RIGHT_SAME_RIGHTS, &outHandle) != ZX_OK,
                           "zx_handle_duplicate failed");
    return outHandle;
}

inline MaybeError CloseHandle(zx_handle_t handle) {
    DAWN_INTERNAL_ERROR_IF(zx_handle_close(handle) != ZX_OK, "zx_handle_close failed");
    return {};
}

#elif DAWN_PLATFORM_IS(POSIX)

constexpr inline int kInvalidHandle = -1;

inline bool IsHandleValid(int handle) {
    return handle >= 0;
}

inline ResultOrError<int> DuplicateHandle(int handle) {
    int outHandle = dup(handle);
    DAWN_INTERNAL_ERROR_IF(outHandle < 0, "dup failed");
    return outHandle;
}

inline MaybeError CloseHandle(int handle) {
    DAWN_INTERNAL_ERROR_IF(close(handle) < 0, "close failed");
    return {};
}

#endif

}  // anonymous namespace

SystemHandle::SystemHandle() : mHandle(kInvalidHandle) {}
SystemHandle::SystemHandle(Handle handle) : mHandle(handle) {}

SystemHandle::SystemHandle(ErrorTag tag) : SystemHandle() {
    dawn::ErrorLog() << "SystemHandle constructed from incorrect handle type.";
    DAWN_ASSERT(false);
}

bool SystemHandle::IsValid() const {
    return IsHandleValid(mHandle);
}

SystemHandle::SystemHandle(SystemHandle&& rhs) {
    mHandle = rhs.mHandle;
    rhs.mHandle = kInvalidHandle;
}

SystemHandle& SystemHandle::operator=(SystemHandle&& rhs) {
    if (this != &rhs) {
        if (IsValid()) {
            Close();
        }
        mHandle = rhs.mHandle;
        rhs.mHandle = kInvalidHandle;
    }
    return *this;
}

SystemHandle::Handle SystemHandle::Get() const {
    return mHandle;
}

SystemHandle::Handle* SystemHandle::GetMut() {
    if (IsValid()) {
        Close();
    }
    return &mHandle;
}

SystemHandle::Handle SystemHandle::Detach() {
    Handle handle = mHandle;
    mHandle = kInvalidHandle;
    return handle;
}

ResultOrError<SystemHandle> SystemHandle::Duplicate() const {
    Handle handle;
    DAWN_TRY_ASSIGN(handle, DuplicateHandle(mHandle));
    return SystemHandle(handle);
}

void SystemHandle::Close() {
    DAWN_ASSERT(IsValid());
    auto result = CloseHandle(mHandle);
    // Still invalidate the handle if Close failed.
    // If Close failed, the handle surely was invalid already.
    mHandle = kInvalidHandle;
    if (DAWN_UNLIKELY(result.IsError())) {
        dawn::ErrorLog() << result.AcquireError()->GetFormattedMessage();
    }
}

SystemHandle::~SystemHandle() {
    if (IsValid()) {
        Close();
    }
}

}  // namespace dawn::native
