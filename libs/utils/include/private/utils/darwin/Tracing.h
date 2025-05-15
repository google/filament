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

#ifndef TNT_UTILS_DARWIN_FILAMENT_TRACING_H
#define TNT_UTILS_DARWIN_FILAMENT_TRACING_H

#include <atomic>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <os/log.h>
#include <os/signpost.h>

#include <utils/compiler.h>
#include <stack>

#if FILAMENT_TRACING_ENABLED == false

#define FILAMENT_TRACING_ENABLE(category)
#define FILAMENT_TRACING_CONTEXT(category)
#define FILAMENT_TRACING_NAME(category, name)
#define FILAMENT_TRACING_FRAME_ID(category, frame)
#define FILAMENT_TRACING_NAME_BEGIN(category, name)
#define FILAMENT_TRACING_NAME_END(category)
#define FILAMENT_TRACING_CALL(category)
#define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie)
#define FILAMENT_TRACING_ASYNC_END(category, name, cookie)
#define FILAMENT_TRACING_VALUE(category, name, val)

#else

// enable tracing
#define FILAMENT_TRACING_ENABLE(category) ::utils::details::Tracing::enable(category)

/**
 * Creates a Tracing context in the current scope. needed for calling all other Tracing
 * commands below.
 */
#define FILAMENT_TRACING_CONTEXT(category) ::utils::details::Tracing ___tracer(category)


// FILAMENT_TRACING_NAME traces the beginning and end of the current scope.  To trace
// the correct start and end times this macro should be declared first in the
// scope body.
// It also automatically creates a Tracing context
#define FILAMENT_TRACING_NAME(category, name) ::utils::details::ScopedTracing ___tracer(name)

// Denotes that a new frame has started processing.
#define FILAMENT_TRACING_FRAME_ID(category, frame) \
    ::utils::details::Tracing(category).frameId(frame)

extern thread_local std::stack<const char*> ___tracerSections;

// FILAMENT_TRACING_CALL is an FILAMENT_TRACING_NAME that uses the current function name.
#define FILAMENT_TRACING_CALL(category) FILAMENT_TRACING_NAME(category, __PRETTY_FUNCTION__)

#define FILAMENT_TRACING_NAME_BEGIN(category, name) \
        ___tracerSections.push(name) , \
        ___tracer.traceBegin(name)

#define FILAMENT_TRACING_NAME_END(category) \
        ___tracer.traceEnd(___tracerSections.top()) , \
        ___tracerSections.pop()

/**
 * Trace the beginning of an asynchronous event. Unlike ATRACE_BEGIN/ATRACE_END
 * contexts, asynchronous events do not need to be nested. The name describes
 * the event, and the cookie provides a unique identifier for distinguishing
 * simultaneous events. The name and cookie used to begin an event must be
 * used to end it.
 */
#define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie) \
        ___tracer.asyncBegin(name, cookie)

/**
 * Trace the end of an asynchronous event.
 * This should have a corresponding FILAMENT_TRACING_ASYNC_BEGIN.
 */
#define FILAMENT_TRACING_ASYNC_END(category, name, cookie) \
        ___tracer.asyncEnd(name, cookie)

/**
 * Traces an integer counter value.  name is used to identify the counter.
 * This can be used to track how a value changes over time.
 */
#define FILAMENT_TRACING_VALUE(category, name, val) \
        ___tracer.value(name, val)

#endif // FILAMENT_TRACING_MODE != FILAMENT_TRACING_MODE_DISABLED

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

namespace utils::details {

class Tracing {
public:
    explicit Tracing(const char* category) noexcept {
        init(category);
    }

    static void enable(const char* category) noexcept;

    void traceBegin(const char* name) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            APPLE_SIGNPOST_EMIT(sGlobalState.tracingLog, OS_SIGNPOST_INTERVAL_BEGIN,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, name)
        }
    }

    void traceEnd(const char* name) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            APPLE_SIGNPOST_EMIT(sGlobalState.tracingLog, OS_SIGNPOST_INTERVAL_END,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, "")
        }
    }

    void asyncBegin(const char* name, int32_t cookie) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            // TODO
        }
    }

    void asyncEnd(const char* name, int32_t cookie) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            // TODO
        }
    }

    void value(const char* name, int32_t value) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64];
            snprintf(buf, 64, "%s - %d", name, value);
            APPLE_SIGNPOST_EMIT(sGlobalState.tracingLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, buf)
        }
    }

    void value(const char* name, int64_t value) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64];
            snprintf(buf, 64, "%s - %lld", name, value);
            APPLE_SIGNPOST_EMIT(sGlobalState.tracingLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, name, buf)
        }
    }

    void frameId(uint32_t frame) noexcept {
        if (UTILS_UNLIKELY(mIsTracingEnabled)) {
            char buf[64];
            snprintf(buf, 64, "frame %u", frame);
            APPLE_SIGNPOST_EMIT(sGlobalState.frameIdLog, OS_SIGNPOST_EVENT,
                    OS_SIGNPOST_ID_EXCLUSIVE, "frame", buf)
        }
    }

private:
    friend class ScopedTracing;

    struct GlobalState {
        std::atomic<uint32_t> isTracingEnabled;

        os_log_t tracingLog;
        os_log_t frameIdLog;
    };

    static GlobalState sGlobalState;

    void init(const char* category) noexcept;

    // cached values for faster access, no need to be initialized
    bool mIsTracingEnabled;

    static void setup() noexcept;
    static void init_once() noexcept;
    static bool isTracingEnabled(uint32_t tag) noexcept;
};

// ------------------------------------------------------------------------------------------------

class ScopedTracing {
public:
    // we don't inline this because it's relatively heavy due to a global check
    ScopedTracing(const char* category, const char* name) noexcept : mTrace(category), mName(name) {
        mTrace.traceBegin(name);
    }

    ~ScopedTracing() noexcept {
        mTrace.traceEnd(mName);
    }

    void value(const char* name, int32_t v) noexcept {
        mTrace.value(name, v);
    }

    void value( const char* name, int64_t v) noexcept {
        mTrace.value(name, v);
    }

private:
    Tracing mTrace;
    const char* mName;
};

} // namespace utils::details


#endif // TNT_UTILS_DARWIN_FILAMENT_TRACING_H
