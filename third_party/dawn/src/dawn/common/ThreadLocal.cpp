// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/common/ThreadLocal.h"

#include <atomic>

#include "absl/base/no_destructor.h"
#include "absl/container/flat_hash_set.h"
#include "dawn/common/MutexProtected.h"

namespace dawn {
namespace {
auto& GetAliveThreads() {
    // Because static variables are destroyed on the thread that starts termination, we need to make
    // sure that this set is available to racing threads during termination. Use absl::NoDestructor
    // to prevent the race on destruction, and creating in a function scope to ensure availability.
    // See documentation here:
    //   https://github.com/abseil/abseil-cpp/blob/master/absl/base/no_destructor.h
    static absl::NoDestructor<MutexProtected<absl::flat_hash_set<ThreadUniqueId>>> sThreadUniqueIds;
    return *sThreadUniqueIds;
}
}  // anonymous namespace

ThreadUniqueId GetThreadUniqueId() {
    static std::atomic<ThreadUniqueId> sNextThreadUniqueId = 1;

    // Each time a thread is started, we assign it a unique Dawn ThreadUniqueId by constructing a
    // ThreadRef. The ThreadRef is a thread_local RAII variable with a destructor that removes the
    // ThreadUniqueId from a globally static set of "running" ThreadUniqueIds. This allows us to
    // check whether a ThreadUniqueId is still running since thread_local variables are destroyed
    // when a thread is terminated.
    struct ThreadRef {
        ThreadUniqueId mThreadUniqueId;

        explicit ThreadRef(ThreadUniqueId ThreadUniqueId) : mThreadUniqueId(ThreadUniqueId) {
            GetAliveThreads()->insert(mThreadUniqueId);
        }
        ~ThreadRef() { GetAliveThreads()->erase(mThreadUniqueId); }
    };
    // This ThreadRef is initialized the first time GetThreadUniqueId is called for each thread (see
    // https://stackoverflow.com/a/49821006), and destroyed when the thread terminates.
    static thread_local ThreadRef sThreadRef(sNextThreadUniqueId++);
    return sThreadRef.mThreadUniqueId;
}

bool IsThreadAlive(ThreadUniqueId ThreadUniqueId) {
    return GetAliveThreads()->count(ThreadUniqueId) != 0u;
}

}  // namespace dawn
