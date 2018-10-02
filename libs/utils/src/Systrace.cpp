/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/Systrace.h>
#include <utils/Log.h>

#if defined(ANDROID)

#include <cinttypes>

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

namespace utils {
namespace details {

/**
 * Maximum size of a message that can be logged to the trace buffer.
 * Note this message includes a tag, the pid, and the string given as the name.
 * Names should be kept short to get the most use of the trace buffer.
 */
#define ATRACE_MESSAGE_LENGTH 512

std::atomic_bool Systrace::sIsTracingReady = { false };
std::atomic<uint32_t> Systrace::sIsTracingEnabled = { 0 };
bool Systrace::sIsTracingAvailable = false;
int Systrace::sMarkerFd = -1;

static pthread_once_t atrace_once_control = PTHREAD_ONCE_INIT;

void Systrace::init_once() noexcept {
    sMarkerFd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
    if (UTILS_UNLIKELY(sMarkerFd == -1)) {
        slog.e << "Error opening trace file: " << strerror(errno) << " (" << errno << ")" << io::endl;
    } else {
        sIsTracingAvailable = true;
    }
    std::atomic_store_explicit(&sIsTracingReady, true, std::memory_order_release);
}

void Systrace::setup() noexcept {
    pthread_once(&atrace_once_control, init_once);
}

void Systrace::enable(uint32_t tags) noexcept {
    init();
    if (UTILS_LIKELY(sIsTracingAvailable)) {
        sIsTracingEnabled.fetch_or(tags, std::memory_order_relaxed);
    }
}

void Systrace::disable(uint32_t tags) noexcept {
    sIsTracingEnabled.fetch_and(~tags, std::memory_order_relaxed);
}

// unfortunately, this generates quite a bit of code because reading a global is not
// trivial. For this reason, we do not inline this method.
bool Systrace::isTracingEnabled(uint32_t tag) noexcept {
    if (tag) {
        init();
        return bool((sIsTracingEnabled.load(std::memory_order_relaxed) | SYSTRACE_TAG_ALWAYS) & tag);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------

void Systrace::begin_body(int fd, int pid, const char* name) noexcept {
    char buf[ATRACE_MESSAGE_LENGTH];

    ssize_t len = snprintf(buf, sizeof(buf), "B|%d|%s", pid, name);
    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }
    write(fd, buf, size_t(len));
}

#define WRITE_MSG(format_begin, format_end, pid, name, value) {                 \
    char buf[ATRACE_MESSAGE_LENGTH];                                            \
    int len = snprintf(buf, sizeof(buf), format_begin "%s" format_end, pid,     \
        name, value);                                                           \
    if (len >= (int) sizeof(buf)) {                                             \
        /* Given the sizeof(buf), and all of the current format buffers,        \
         * it is impossible for name_len to be < 0 if len >= sizeof(buf). */    \
        int name_len = strlen(name) - (len - sizeof(buf)) - 1;                  \
        /* Truncate the name to make the message fit. */                        \
        slog.e << "Truncated name in " << __FUNCTION__ << ": " << name << io::endl; \
        len = snprintf(buf, sizeof(buf), format_begin "%.*s" format_end, pid,   \
            name_len, name, value);                                             \
    }                                                                           \
    write(fd, buf, len);                                                        \
}

void Systrace::async_begin_body(int fd, int pid, const char* name, int32_t cookie) noexcept {
    WRITE_MSG("S|%d|", "|%" PRId32, pid, name, cookie);
}

void Systrace::async_end_body(int fd, int pid, const char* name, int32_t cookie) noexcept {
    WRITE_MSG("F|%d|", "|%" PRId32, pid, name, cookie);
}

void Systrace::int_body(int fd, int pid, const char* name, int32_t value) noexcept {
    WRITE_MSG("C|%d|", "|%" PRId32, pid, name, value);
}

void Systrace::int64_body(int fd, int pid, const char* name, int64_t value) noexcept {
    WRITE_MSG("C|%d|", "|%" PRId64, pid, name, value);
}

// ------------------------------------------------------------------------------------------------

void Systrace::init(uint32_t tag) noexcept {
    // must be called first
    mIsTracingEnabled = isTracingEnabled(tag);

    // these are now initialized
    mMarkerFd = sMarkerFd;

    mPid = getpid();
}

} // namespace details
} // namespace utils

#endif // ANDROID
