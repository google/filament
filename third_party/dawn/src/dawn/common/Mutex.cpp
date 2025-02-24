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

#include "dawn/common/Mutex.h"

namespace dawn {
Mutex::~Mutex() = default;

void Mutex::Lock() {
#if defined(DAWN_ENABLE_ASSERTS)
    auto currentThread = std::this_thread::get_id();
    DAWN_ASSERT(mOwner.load(std::memory_order_acquire) != currentThread);
#endif  // DAWN_ENABLE_ASSERTS

    mNativeMutex.lock();

#if defined(DAWN_ENABLE_ASSERTS)
    mOwner.store(currentThread, std::memory_order_release);
#endif  // DAWN_ENABLE_ASSERTS
}
void Mutex::Unlock() {
#if defined(DAWN_ENABLE_ASSERTS)
    DAWN_ASSERT(IsLockedByCurrentThread());
    mOwner.store(std::thread::id(), std::memory_order_release);
#endif  // DAWN_ENABLE_ASSERTS

    mNativeMutex.unlock();
}

bool Mutex::IsLockedByCurrentThread() {
#if defined(DAWN_ENABLE_ASSERTS)
    return mOwner.load(std::memory_order_acquire) == std::this_thread::get_id();
#else
    // This is not supported.
    DAWN_CHECK(false);
#endif
}
}  // namespace dawn
