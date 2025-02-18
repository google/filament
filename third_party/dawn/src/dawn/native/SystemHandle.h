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

#ifndef SRC_DAWN_NATIVE_SYSTEMHANDLE_H_
#define SRC_DAWN_NATIVE_SYSTEMHANDLE_H_

#include <utility>

#include "dawn/common/NonCopyable.h"
#include "dawn/common/Platform.h"
#include "dawn/native/Error.h"

namespace dawn::native {

class SystemHandle : public NonCopyable {
  public:
    // Create an alias for the actual primitive handle type on a platform.
#if DAWN_PLATFORM_IS(WINDOWS)
    using Handle = void*;
#elif DAWN_PLATFORM_IS(FUCHSIA)
    using Handle = uint32_t;
#elif DAWN_PLATFORM_IS(POSIX)
    using Handle = int;
#else
#error "Platform not supported."
#endif

    SystemHandle();

    // Create a SystemHandle by taking ownership of `handle`.
    // Declaration is a template so that there is never implicit
    // conversion of the handle arg.
    template <typename Arg>
    static SystemHandle Acquire(Arg handle) {
        return SystemHandle(handle);
    }

    // Create a SystemHandle by duplicating `handle`.
    // Declaration is a template so that there is never implicit
    // conversion of the handle arg.
    template <typename Arg>
    static ResultOrError<SystemHandle> Duplicate(Arg handle) {
        SystemHandle tmpOwnedHandle = SystemHandle::Acquire(handle);
        SystemHandle dupHandle;
        DAWN_TRY_ASSIGN(dupHandle, tmpOwnedHandle.Duplicate());
        tmpOwnedHandle.Detach();
        return ResultOrError<SystemHandle>{std::move(dupHandle)};
    }

    bool IsValid() const;

    SystemHandle(SystemHandle&& rhs);
    SystemHandle& operator=(SystemHandle&& rhs);

    Handle Get() const;
    Handle* GetMut();
    Handle Detach();
    ResultOrError<SystemHandle> Duplicate() const;
    void Close();

    ~SystemHandle();

  private:
    struct ErrorTag {};

    explicit SystemHandle(Handle handle);
    explicit SystemHandle(ErrorTag tag);

    // Constructor when the type does not match the Handle type on this platform.
    template <typename Arg, typename = std::enable_if_t<!std::is_same_v<Arg, Handle>>>
    explicit SystemHandle(Arg) : SystemHandle(ErrorTag{}) {}

    void LogIncorrectHandleType();

    Handle mHandle;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SYSTEMHANDLE_H_
