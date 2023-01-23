/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_UTILS_DARWIN_SYSTRACE_H
#define TNT_UTILS_DARWIN_SYSTRACE_H

#include <atomic>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <os/log.h>
#include <os/signpost.h>

#include <utils/compiler.h>
#include <stack>

// enable tracing
#define SYSTRACE_ENABLE() ::utils::details::Systrace::enable(SYSTRACE_TAG)

// disable tracing
#define SYSTRACE_DISABLE() ::utils::details::Systrace::disable(SYSTRACE_TAG)


/**
 * Creates a Systrace context in the current scope. needed for calling all other systrace
 * commands below.
 */
#define SYSTRACE_CONTEXT() ::utils::details::Systrace ___tracer(SYSTRACE_TAG)


// SYSTRACE_NAME traces the beginning and end of the current scope.  To trace
// the correct start and end times this macro should be declared first in the
// scope body.
// It also automatically creates a Systrace context
#define SYSTRACE_NAME(name) ::utils::details::ScopedTrace ___tracer(SYSTRACE_TAG, name)

// Denotes that a new frame has started processing.
#define SYSTRACE_FRAME_ID(frame) \
    ::utils::details::Systrace(SYSTRACE_TAG).frameId(SYSTRACE_TAG, frame)

extern thread_local std::stack<const char*> ___tracerSections;

// SYSTRACE_CALL is an SYSTRACE_NAME that uses the current function name.
#define SYSTRACE_CALL() SYSTRACE_NAME(__PRETTY_FUNCTION__)

#define SYSTRACE_NAME_BEGIN(name) \
        ___tracerSections.push(name) , \
        ___tracer.traceBegin(SYSTRACE_TAG, name)

#define SYSTRACE_NAME_END() \
        ___tracer.traceEnd(SYSTRACE_TAG, ___tracerSections.top()) , \
        ___tracerSections.pop()

/**
 * Trace the beginning of an asynchronous event. Unlike ATRACE_BEGIN/ATRACE_END
 * contexts, asynchronous events do not need to be nested. The name describes
 * the event, and the cookie provides a unique identifier for distinguishing
 * simultaneous events. The name and cookie used to begin an event must be
 * used to end it.
 */
#define SYSTRACE_ASYNC_BEGIN(name, cookie) \
        ___tracer.asyncBegin(SYSTRACE_TAG, name, cookie)

/**
 * Trace the end of an asynchronous event.
 * This should have a corresponding SYSTRACE_ASYNC_BEGIN.
 */
#define SYSTRACE_ASYNC_END(name, cookie) \
        ___tracer.asyncEnd(SYSTRACE_TAG, name, cookie)

/**
 * Traces an integer counter value.  name is used to identify the counter.
 * This can be used to track how a value changes over time.
 */
#define SYSTRACE_VALUE32(name, val) \
        ___tracer.value(SYSTRACE_TAG, name, int32_t(val))

#define SYSTRACE_VALUE64(name, val) \
        ___tracer.value(SYSTRACE_TAG, name, int64_t(val))

// ------------------------------------------------------------------------------------------------
// No user serviceable code below...
// ------------------------------------------------------------------------------------------------

// This is an alternative to os_signpost_emit_with_type that allows non-compile time strings (namely
// for us, __FUNCTION__).
// The trade-off is that this doesn't allow messages to have printf-style formatting.
// It's fine to pass an empty string to __builtin_os_log_format_buffer_size and
// __builtin_os_log_format, because they return the same value for strings without any format
// specifiers.
// This is fragile, so should only be used to assist debugging and never in production.
#define APPLE_SIGNPOST_EMIT(log, type, spid, name, message) \
    if (os_signpost_enabled(log)) { \
        uint8_t _os_fmt_buf[__builtin_os_log_format_buffer_size("")]; \
        _os_signpost_emit_with_name_impl( \
                &__dso_handle, log, type, spid, \
                name, message, \
                (uint8_t *)__builtin_os_log_format(_os_fmt_buf, ""), \
                (uint32_t)sizeof(_os_fmt_buf)); \
    }

namespace utils {
namespace details {

class Systrace {
   public:

    enum tags {
        NEVER       = SYSTRACE_TAG_NEVER,
        ALWAYS      = SYSTRACE_TAG_ALWAYS,
        FILAMENT    = SYSTRACE_TAG_FILAMENT,
        JOBSYSTEM   = SYSTRACE_TAG_JOBSYSTEM
        // we could define more TAGS here, as we need them.
    };

    explicit Systrace(uint32_t tag) noexcept {
        if (tag) init(tag);
    }

    static void enable(uint32_t tags) noexcept;
    static void disable(uint32_t tags) noexcept;

    inline void traceBegin(uint32_t tag, const char* name) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            APPLE_SIGNPOST_EMIT(sGlobalState.systraceLog, OS_SIGNPOST_INTERVAL_BEGIN,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, name)
        }
    }

    inline void traceEnd(uint32_t tag, const char* name) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            APPLE_SIGNPOST_EMIT(sGlobalState.systraceLog, OS_SIGNPOST_INTERVAL_END,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, "")
        }
    }

    inline void asyncBegin(uint32_t tag, const char* name, int32_t cookie) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            // TODO
        }
    }

    inline void asyncEnd(uint32_t tag, const char* name, int32_t cookie) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            // TODO
        }
    }

    inline void value(uint32_t tag, const char* name, int32_t value) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64];
            snprintf(buf, 64, "%s - %d", name, value);
            APPLE_SIGNPOST_EMIT(sGlobalState.systraceLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, buf)
        }
    }

    inline void value(uint32_t tag, const char* name, int64_t value) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64];
            snprintf(buf, 64, "%s - %lld", name, value);
            APPLE_SIGNPOST_EMIT(sGlobalState.systraceLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, buf)
        }
    }

    inline void frameId(uint32_t tag, uint32_t frame) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64]; \
            snprintf(buf, 64, "frame %u", frame); \
            APPLE_SIGNPOST_EMIT(sGlobalState.frameIdLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, "frame", buf)
        }
    }

   private:
    friend class ScopedTrace;

    struct GlobalState {
        std::atomic<uint32_t> isTracingEnabled;

        os_log_t systraceLog;
        os_log_t frameIdLog;
    };

    static GlobalState sGlobalState;

    void init(uint32_t tag) noexcept;

    // cached values for faster access, no need to be initialized
    bool mIsTracingEnabled;

    static void setup() noexcept;
    static void init_once() noexcept;
    static bool isTracingEnabled(uint32_t tag) noexcept;
};

// ------------------------------------------------------------------------------------------------

class ScopedTrace {
   public:
    // we don't inline this because it's relatively heavy due to a global check
    ScopedTrace(uint32_t tag, const char* name) noexcept : mTrace(tag), mName(name), mTag(tag) {
        mTrace.traceBegin(tag, name);
    }

    inline ~ScopedTrace() noexcept {
        mTrace.traceEnd(mTag, mName);
    }

    inline void value(uint32_t tag, const char* name, int32_t v) noexcept {
        mTrace.value(tag, name, v);
    }

    inline void value(uint32_t tag, const char* name, int64_t v) noexcept {
        mTrace.value(tag, name, v);
    }

   private:
    Systrace mTrace;
    const char* mName;
    const uint32_t mTag;
};

} // namespace details
} // namespace utils

#endif // TNT_UTILS_DARWIN_SYSTRACE_H
