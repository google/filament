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

#ifndef SRC_DAWN_COMMON_MUTEX_H_
#define SRC_DAWN_COMMON_MUTEX_H_

#include <atomic>
#include <mutex>
#include <thread>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"

namespace dawn {
class Mutex : public RefCounted, NonCopyable {
  public:
    template <typename MutexRef>
    struct AutoLockBase : NonMovable {
        AutoLockBase() : mMutex(nullptr) {}
        explicit AutoLockBase(MutexRef mutex) : mMutex(std::move(mutex)) {
            if (mMutex != nullptr) {
                mMutex->Lock();
            }
        }
        ~AutoLockBase() {
            if (mMutex != nullptr) {
                mMutex->Unlock();
            }
        }

      private:
        MutexRef mMutex;
    };

    // This scoped lock won't keep the mutex alive.
    using AutoLock = AutoLockBase<Mutex*>;
    // This scoped lock will keep the mutex alive until out of scope.
    using AutoLockAndHoldRef = AutoLockBase<Ref<Mutex>>;

    ~Mutex() override;

    void Lock();
    void Unlock();

    // This method is only enabled when DAWN_ENABLE_ASSERTS is turned on.
    // Thus it should only be wrapped inside DAWN_ASSERT() macro.
    // i.e. DAWN_ASSERT(mutex.IsLockedByCurrentThread())
    bool IsLockedByCurrentThread();

  private:
#if defined(DAWN_ENABLE_ASSERTS)
    std::atomic<std::thread::id> mOwner;
#endif  // DAWN_ENABLE_ASSERTS
    std::mutex mNativeMutex;
};

}  // namespace dawn

#endif
