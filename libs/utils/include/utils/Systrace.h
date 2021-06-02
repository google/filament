/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_SYSTRACE_H
#define TNT_UTILS_SYSTRACE_H


#define SYSTRACE_TAG_NEVER          (0)
#define SYSTRACE_TAG_ALWAYS         (1<<0)
#define SYSTRACE_TAG_FILAMENT       (1<<1)  // don't change, used in makefiles
#define SYSTRACE_TAG_JOBSYSTEM      (1<<2)


#if defined(ANDROID)

#include <atomic>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <utils/compiler.h>

/*
 * The SYSTRACE_ macros use SYSTRACE_TAG as a the TAG, which should be defined
 * before this file is included. If not, the SYSTRACE_TAG_ALWAYS tag will be used.
 */

#ifndef SYSTRACE_TAG
#define SYSTRACE_TAG (SYSTRACE_TAG_ALWAYS)
#endif

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

// SYSTRACE_CALL is an SYSTRACE_NAME that uses the current function name.
#define SYSTRACE_CALL() SYSTRACE_NAME(__FUNCTION__)

#define SYSTRACE_NAME_BEGIN(name) \
        ___tracer.traceBegin(SYSTRACE_TAG, name)

#define SYSTRACE_NAME_END() \
        ___tracer.traceEnd(SYSTRACE_TAG)


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

    Systrace(uint32_t tag) noexcept {
        if (tag) init(tag);
    }

    static void enable(uint32_t tags) noexcept;
    static void disable(uint32_t tags) noexcept;


    inline void traceBegin(uint32_t tag, const char* name) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            beginSection(this, name);
        }
    }

    inline void traceEnd(uint32_t tag) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            endSection(this);
        }
    }

    inline void asyncBegin(uint32_t tag, const char* name, int32_t cookie) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            beginAsyncSection(this, name, cookie);
        }
    }

    inline void asyncEnd(uint32_t tag, const char* name, int32_t cookie) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            endAsyncSection(this, name, cookie);
        }
    }

    inline void value(uint32_t tag, const char* name, int32_t value) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            setCounter(this, name, value);
        }
    }

    inline void value(uint32_t tag, const char* name, int64_t value) noexcept {
        if (tag && UTILS_UNLIKELY(mIsTracingEnabled)) {
            setCounter(this, name, value);
        }
    }

private:
    friend class ScopedTrace;

    // whether tracing is supported at all by the platform

    using ATrace_isEnabled_t          = bool (*)(void);
    using ATrace_beginSection_t       = void (*)(const char* sectionName);
    using ATrace_endSection_t         = void (*)(void);
    using ATrace_beginAsyncSection_t  = void (*)(const char* sectionName, int32_t cookie);
    using ATrace_endAsyncSection_t    = void (*)(const char* sectionName, int32_t cookie);
    using ATrace_setCounter_t         = void (*)(const char* counterName, int64_t counterValue);

    struct GlobalState {
        bool isTracingAvailable;
        std::atomic<uint32_t> isTracingEnabled;
        int markerFd;

        ATrace_isEnabled_t ATrace_isEnabled;
        ATrace_beginSection_t ATrace_beginSection;
        ATrace_endSection_t ATrace_endSection;
        ATrace_beginAsyncSection_t ATrace_beginAsyncSection;
        ATrace_endAsyncSection_t ATrace_endAsyncSection;
        ATrace_setCounter_t ATrace_setCounter;

        void (*beginSection)(Systrace* that, const char* name);
        void (*endSection)(Systrace* that);
        void (*beginAsyncSection)(Systrace* that, const char* name, int32_t cookie);
        void (*endAsyncSection)(Systrace* that, const char* name, int32_t cookie);
        void (*setCounter)(Systrace* that, const char* name, int64_t value);
    };

    static GlobalState sGlobalState;


    // per-instance versions for better performance
    ATrace_isEnabled_t ATrace_isEnabled;
    ATrace_beginSection_t ATrace_beginSection;
    ATrace_endSection_t ATrace_endSection;
    ATrace_beginAsyncSection_t ATrace_beginAsyncSection;
    ATrace_endAsyncSection_t ATrace_endAsyncSection;
    ATrace_setCounter_t ATrace_setCounter;

    void (*beginSection)(Systrace* that, const char* name);
    void (*endSection)(Systrace* that);
    void (*beginAsyncSection)(Systrace* that, const char* name, int32_t cookie);
    void (*endAsyncSection)(Systrace* that, const char* name, int32_t cookie);
    void (*setCounter)(Systrace* that, const char* name, int64_t value);

    void init(uint32_t tag) noexcept;

    // cached values for faster access, no need to be initialized
    bool mIsTracingEnabled;
    int mMarkerFd = -1;
    pid_t mPid;

    static void setup() noexcept;
    static void init_once() noexcept;
    static bool isTracingEnabled(uint32_t tag) noexcept;

    static void begin_body(int fd, int pid, const char* name) noexcept;
    static void end_body(int fd, int pid) noexcept;
    static void async_begin_body(int fd, int pid, const char* name, int32_t cookie) noexcept;
    static void async_end_body(int fd, int pid, const char* name, int32_t cookie) noexcept;
    static void int64_body(int fd, int pid, const char* name, int64_t value) noexcept;
};

// ------------------------------------------------------------------------------------------------

class ScopedTrace {
public:
    // we don't inline this because it's relatively heavy due to a global check
    ScopedTrace(uint32_t tag, const char* name) noexcept : mTrace(tag), mTag(tag) {
        mTrace.traceBegin(tag, name);
    }

    inline ~ScopedTrace() noexcept {
        mTrace.traceEnd(mTag);
    }

    inline void value(uint32_t tag, const char* name, int32_t v) noexcept {
        mTrace.value(tag, name, v);
    }

    inline void value(uint32_t tag, const char* name, int64_t v) noexcept {
        mTrace.value(tag, name, v);
    }

private:
    Systrace mTrace;
    const uint32_t mTag;
};

} // namespace details
} // namespace utils

// ------------------------------------------------------------------------------------------------
#else // !ANDROID
// ------------------------------------------------------------------------------------------------

#define SYSTRACE_ENABLE()
#define SYSTRACE_DISABLE()
#define SYSTRACE_CONTEXT()
#define SYSTRACE_NAME(name)
#define SYSTRACE_NAME_BEGIN(name)
#define SYSTRACE_NAME_END()
#define SYSTRACE_CALL()
#define SYSTRACE_ASYNC_BEGIN(name, cookie)
#define SYSTRACE_ASYNC_END(name, cookie)
#define SYSTRACE_VALUE32(name, val)
#define SYSTRACE_VALUE64(name, val)

#endif // ANDROID

#endif // TNT_UTILS_SYSTRACE_H
