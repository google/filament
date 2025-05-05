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

#include <cinttypes>

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <dlfcn.h>

namespace utils {
namespace details {

static pthread_once_t atrace_once_control = PTHREAD_ONCE_INIT;

template <typename T>
static void loadSymbol(T*& pfn, const char *symbol) noexcept {
    pfn = (T*)dlsym(RTLD_DEFAULT, symbol);
}

Systrace::GlobalState Systrace::sGlobalState = {};

void Systrace::init_once() noexcept {
    GlobalState& s = sGlobalState;

    s.markerFd = -1;

    // API 23
    loadSymbol(s.ATrace_isEnabled, "ATrace_isEnabled");
    loadSymbol(s.ATrace_beginSection, "ATrace_beginSection");
    loadSymbol(s.ATrace_endSection, "ATrace_endSection");
    // API 29
    loadSymbol(s.ATrace_beginAsyncSection, "ATrace_beginAsyncSection");
    loadSymbol(s.ATrace_endAsyncSection, "ATrace_endAsyncSection");
    loadSymbol(s.ATrace_setCounter, "ATrace_setCounter");


    const bool hasBasicAtrace = s.ATrace_isEnabled &&
                                s.ATrace_beginSection &&
                                s.ATrace_endSection;

    const bool hasFullATrace = hasBasicAtrace &&
                               s.ATrace_beginAsyncSection &&
                               s.ATrace_endAsyncSection &&
                               s.ATrace_setCounter;

    if (!hasFullATrace) {
        s.markerFd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
    }

    if (hasBasicAtrace && !hasFullATrace) {
        // no-op if we don't have all these
        s.ATrace_beginAsyncSection = [](const char* sectionName, int32_t cookie){};
        s.ATrace_endAsyncSection = [](const char* sectionName, int32_t cookie){};
        s.ATrace_setCounter = [](const char* sectionName, int64_t counterValue){};
    }

    const bool hasLegacySystrace = s.markerFd != -1;

    if (hasLegacySystrace && !hasFullATrace) {
        // use legacy
        s.beginSection = [](Systrace* that, const char* name) {
            begin_body(that->mMarkerFd, that->mPid, name);
        };
        s.endSection = [](Systrace* that) {
            end_body(that->mMarkerFd, that->mPid);
        };
        s.beginAsyncSection = [](Systrace* that, const char* name, int32_t cookie) {
            async_begin_body(that->mMarkerFd, that->mPid, name, cookie);
        };
        s.endAsyncSection = [](Systrace* that, const char* name, int32_t cookie) {
            async_end_body(that->mMarkerFd, that->mPid, name, cookie);
        };
        s.setCounter = [](Systrace* that, const char* name, int64_t value) {
            int64_body(that->mMarkerFd, that->mPid, name, value);
        };
    } else if (hasBasicAtrace) {
        // we have at least basic ATrace
        s.beginSection = [](Systrace* that, const char* name) {
            that->ATrace_beginSection(name);
        };
        s.endSection = [](Systrace* that) {
            that->ATrace_endSection();
        };
        s.beginAsyncSection = [](Systrace* that, const char* name, int32_t cookie) {
            that->ATrace_beginAsyncSection(name, cookie);
        };
        s.endAsyncSection = [](Systrace* that, const char* name, int32_t cookie) {
            that->ATrace_endAsyncSection(name, cookie);
        };
        s.setCounter = [](Systrace* that, const char* name, int64_t value) {
            that->ATrace_setCounter(name, value);
        };
    }

    s.isTracingAvailable = hasLegacySystrace || hasFullATrace || hasBasicAtrace;
}

void Systrace::setup() noexcept {
    pthread_once(&atrace_once_control, init_once);
}

void Systrace::enable(uint32_t tags) noexcept {
    setup();
    if (UTILS_LIKELY(sGlobalState.isTracingAvailable)) {
        sGlobalState.isTracingEnabled.fetch_or(tags, std::memory_order_relaxed);
    }
}

void Systrace::disable(uint32_t tags) noexcept {
    sGlobalState.isTracingEnabled.fetch_and(~tags, std::memory_order_relaxed);
}

// unfortunately, this generates quite a bit of code because reading a global is not
// trivial. For this reason, we do not inline this method.
bool Systrace::isTracingEnabled(uint32_t tag) noexcept {
    if (tag) {
        setup();
        return bool((sGlobalState.isTracingEnabled.load(std::memory_order_relaxed) | SYSTRACE_TAG_ALWAYS) & tag);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------

void Systrace::init(uint32_t tag) noexcept {
    // must be called first
    mIsTracingEnabled = isTracingEnabled(tag);

    // cache static variables for better efficiency
    GlobalState& s = sGlobalState;
    ATrace_isEnabled         = s.ATrace_isEnabled;
    ATrace_beginSection      = s.ATrace_beginSection;
    ATrace_endSection        = s.ATrace_endSection;
    ATrace_beginAsyncSection = s.ATrace_beginAsyncSection;
    ATrace_endAsyncSection   = s.ATrace_endAsyncSection;
    ATrace_setCounter        = s.ATrace_setCounter;

    beginSection = s.beginSection;
    endSection = s.endSection;
    beginAsyncSection = s.beginAsyncSection;
    endAsyncSection = s.endAsyncSection;
    setCounter = s.setCounter;

    mMarkerFd = s.markerFd;

    mPid = getpid();
}

// ------------------------------------------------------------------------------------------------

/**
 * Maximum size of a message that can be logged to the trace buffer.
 * Note this message includes a tag, the pid, and the string given as the name.
 * Names should be kept short to get the most use of the trace buffer.
 */
#define ATRACE_MESSAGE_LENGTH 512

#define WRITE_MSG(format_begin, format_end, pid, name, value) {                 \
    char buf[ATRACE_MESSAGE_LENGTH];                                            \
    int len = snprintf(buf, sizeof(buf), format_begin "%s" format_end, pid,     \
        name, value);                                                           \
    if (len >= (int) sizeof(buf)) {                                             \
        /* Given the sizeof(buf), and all of the current format buffers,        \
         * it is impossible for name_len to be < 0 if len >= sizeof(buf). */    \
        int name_len = strlen(name) - (len - sizeof(buf)) - 1;                  \
        /* Truncate the name to make the message fit. */                        \
        len = snprintf(buf, sizeof(buf), format_begin "%.*s" format_end, pid,   \
            name_len, name, value);                                             \
    }                                                                           \
    write(fd, buf, len);                                                        \
}

void Systrace::begin_body(int fd, int pid, const char* name) noexcept {
    char buf[ATRACE_MESSAGE_LENGTH];
    ssize_t len = snprintf(buf, sizeof(buf), "B|%d|%s", pid, name);
    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }
    write(fd, buf, size_t(len));
}

void Systrace::end_body(int fd, int pid) noexcept {
    const char END_TAG = 'E';
    write(fd, &END_TAG, 1);
}

void Systrace::async_begin_body(int fd, int pid, const char* name, int32_t cookie) noexcept {
    WRITE_MSG("S|%d|", "|%" PRId32, pid, name, cookie);
}

void Systrace::async_end_body(int fd, int pid, const char* name, int32_t cookie) noexcept {
    WRITE_MSG("F|%d|", "|%" PRId32, pid, name, cookie);
}

void Systrace::int64_body(int fd, int pid, const char* name, int64_t value) noexcept {
    WRITE_MSG("C|%d|", "|%" PRId64, pid, name, value);
}

} // namespace details
} // namespace utils
