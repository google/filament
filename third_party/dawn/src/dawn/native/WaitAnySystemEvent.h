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

#ifndef SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_
#define SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_

#include <limits>
#include <utility>

#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#elif DAWN_PLATFORM_IS(FUCHSIA)
#include <poll.h>
#include <unistd.h>
#elif DAWN_PLATFORM_IS(POSIX)
#include <sys/poll.h>
#include <unistd.h>
#endif

#include "absl/container/inlined_vector.h"
#include "dawn/common/Log.h"
#include "dawn/native/SystemEvent.h"

namespace dawn::native {

template <typename T, T Infinity>
T ToMillisecondsGeneric(Nanoseconds timeout) {
    uint64_t ns = uint64_t{timeout};
    uint64_t ms = 0;
    if (ns > 0) {
        ms = (ns - 1) / 1'000'000 + 1;
        if (ms > std::numeric_limits<T>::max()) {
            return Infinity;  // Round long timeout up to infinity
        }
    }
    return static_cast<T>(ms);
}

#if DAWN_PLATFORM_IS(WINDOWS)
#define ToMilliseconds ToMillisecondsGeneric<DWORD, INFINITE>
#elif DAWN_PLATFORM_IS(POSIX)
#define ToMilliseconds ToMillisecondsGeneric<int, -1>
#endif

// WaitAnySystemEvent on an iterator range converts those iterators to the
// platform-specific wait handles, and then waits on them.
template <typename It>
[[nodiscard]] bool WaitAnySystemEvent(It begin, It end, Nanoseconds timeout) {
    static_assert(std::is_same_v<typename std::iterator_traits<It>::value_type,
                                 std::pair<const SystemEventReceiver&, bool*>>);
    size_t count = std::distance(begin, end);
    if (count == 0) {
        return false;
    }
#if DAWN_PLATFORM_IS(WINDOWS)
    absl::InlinedVector<HANDLE, 4 /* avoid heap allocation for small waits */> handles;
    handles.reserve(count);
    for (auto it = begin; it != end; ++it) {
        handles.push_back((*it).first.mPrimitive.Get());
    }
    DAWN_ASSERT(handles.size() <= MAXIMUM_WAIT_OBJECTS);
    DWORD status = WaitForMultipleObjects(handles.size(), handles.data(), /*bWaitAll=*/false,
                                          ToMilliseconds(timeout));
    if (status == WAIT_TIMEOUT) {
        return false;
    }
    DAWN_CHECK(WAIT_OBJECT_0 <= status && status < WAIT_OBJECT_0 + count);
    const size_t completedIndex = status - WAIT_OBJECT_0;

    *(*(begin + completedIndex)).second = true;
    return true;
#elif DAWN_PLATFORM_IS(POSIX)
    absl::InlinedVector<pollfd, 4 /* avoid heap allocation for small waits */> pollfds;
    pollfds.reserve(count);
    for (auto it = begin; it != end; ++it) {
        pollfds.push_back(pollfd{static_cast<int>((*it).first.mPrimitive.Get()), POLLIN, 0});
    }
    int status;
    bool retry;
    do {
        retry = false;
        status = poll(pollfds.data(), pollfds.size(), ToMilliseconds(timeout));
        if (status < 0) {
            int lErrno = errno;
            if (EAGAIN == lErrno || EINTR == lErrno) {
                retry = true;
            } else {
                dawn::ErrorLog() << "poll errno=" << lErrno;
            }
        }
    } while (retry);

    DAWN_CHECK(status >= 0);
    if (status == 0) {
        return false;
    }

    size_t i = 0;
    for (auto it = begin; it != end; ++it, ++i) {
        int revents = pollfds[i].revents;
        static constexpr int kAllowedEvents = POLLIN | POLLHUP;
        DAWN_CHECK((revents & kAllowedEvents) == revents);

        bool ready = (pollfds[i].revents & POLLIN) != 0;
        *(*it).second = ready;
    }

    return true;
#else
    DAWN_CHECK(false);  // Not implemented.
#endif
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_WAITANYSYSTEMEVENT_H_
