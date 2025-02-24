//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.h: Debugging utilities. A lot of the logging code is adapted from Chromium's
// base/logging.h.

#ifndef COMMON_DEBUG_H_
#define COMMON_DEBUG_H_

#include <assert.h>
#include <stdio.h>

#include <iomanip>
#include <ios>
#include <mutex>
#include <sstream>
#include <string>

#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "common/entry_points_enum_autogen.h"
#include "common/log_utils.h"
#include "common/platform.h"

#if defined(ANGLE_PLATFORM_WINDOWS)
#    include <sal.h>
typedef unsigned long DWORD;
typedef _Return_type_success_(return >= 0) long HRESULT;
#endif

#if !defined(TRACE_OUTPUT_FILE)
#    define TRACE_OUTPUT_FILE "angle_debug.txt"
#endif

namespace gl
{
class Context;

// Pairs a begin event with an end event.
class [[nodiscard]] ScopedPerfEventHelper : angle::NonCopyable
{
  public:
    ScopedPerfEventHelper(Context *context, angle::EntryPoint entryPoint);
    ~ScopedPerfEventHelper();
    ANGLE_FORMAT_PRINTF(2, 3)
    void begin(const char *format, ...);

  private:
    gl::Context *mContext;
    const angle::EntryPoint mEntryPoint;
    const char *mFunctionName;
    bool mCalledBeginEvent;
};

// Wraps the API/Platform-specific debug annotation functions.
// Also handles redirecting logging destination.
class DebugAnnotator : angle::NonCopyable
{
  public:
    DebugAnnotator() {}
    virtual ~DebugAnnotator() {}
    virtual void beginEvent(gl::Context *context,
                            angle::EntryPoint entryPoint,
                            const char *eventName,
                            const char *eventMessage)                    = 0;
    virtual void endEvent(gl::Context *context,
                          const char *eventName,
                          angle::EntryPoint entryPoint)                  = 0;
    virtual void setMarker(gl::Context *context, const char *markerName) = 0;
    virtual bool getStatus(const gl::Context *context)                   = 0;
    // Log Message Handler that gets passed every log message,
    // when debug annotations are initialized,
    // replacing default handling by LogMessage.
    virtual void logMessage(const LogMessage &msg) const = 0;
};

void InitializeDebugAnnotations(DebugAnnotator *debugAnnotator);
void UninitializeDebugAnnotations();
bool DebugAnnotationsActive(const gl::Context *context);
bool DebugAnnotationsInitialized();

void InitializeDebugMutexIfNeeded();

angle::SimpleMutex &GetDebugMutex();
}  // namespace gl

// The state tracked by ANGLE will be validated with the driver state before each call
#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#    define ANGLE_STATE_VALIDATION_ENABLED
#endif

#if defined(__GNUC__)
#    define ANGLE_CRASH() __builtin_trap()
#else
#    define ANGLE_CRASH() ((void)(*(volatile char *)0 = 0)), __assume(0)
#endif

#define ANGLE_UNUSED_VARIABLE(variable) (static_cast<void>(variable))

// Defining ANGLE_ENABLE_STRUCT_PADDING_WARNINGS will enable warnings when members are added to
// structs to enforce packing. This is helpful for diagnosing unexpected struct sizes when making
// fast cache variables.
#if defined(__clang__)
#    define ANGLE_ENABLE_STRUCT_PADDING_WARNINGS \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic error \"-Wpadded\"")
#    define ANGLE_DISABLE_STRUCT_PADDING_WARNINGS _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#    define ANGLE_ENABLE_STRUCT_PADDING_WARNINGS \
        _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic error \"-Wpadded\"")
#    define ANGLE_DISABLE_STRUCT_PADDING_WARNINGS _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#    define ANGLE_ENABLE_STRUCT_PADDING_WARNINGS \
        __pragma(warning(push)) __pragma(warning(error : 4820))
#    define ANGLE_DISABLE_STRUCT_PADDING_WARNINGS __pragma(warning(pop))
#else
#    define ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
#    define ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_SUGGEST_OVERRIDE_WARNINGS                               \
        _Pragma("clang diagnostic push")                                          \
            _Pragma("clang diagnostic ignored \"-Wsuggest-destructor-override\"") \
                _Pragma("clang diagnostic ignored \"-Wsuggest-override\"")
#    define ANGLE_REENABLE_SUGGEST_OVERRIDE_WARNINGS _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_SUGGEST_OVERRIDE_WARNINGS
#    define ANGLE_REENABLE_SUGGEST_OVERRIDE_WARNINGS
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_EXTRA_SEMI_WARNING \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wextra-semi\"")
#    define ANGLE_REENABLE_EXTRA_SEMI_WARNING _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_EXTRA_SEMI_WARNING
#    define ANGLE_REENABLE_EXTRA_SEMI_WARNING
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_EXTRA_SEMI_STMT_WARNING \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wextra-semi-stmt\"")
#    define ANGLE_REENABLE_EXTRA_SEMI_STMT_WARNING _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_EXTRA_SEMI_STMT_WARNING
#    define ANGLE_REENABLE_EXTRA_SEMI_STMT_WARNING
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_SHADOWING_WARNING \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wshadow-field\"")
#    define ANGLE_REENABLE_SHADOWING_WARNING _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_SHADOWING_WARNING
#    define ANGLE_REENABLE_SHADOWING_WARNING
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_DESTRUCTOR_OVERRIDE_WARNING \
        _Pragma("clang diagnostic push")              \
            _Pragma("clang diagnostic ignored \"-Winconsistent-missing-destructor-override\"")
#    define ANGLE_REENABLE_DESTRUCTOR_OVERRIDE_WARNING _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_DESTRUCTOR_OVERRIDE_WARNING
#    define ANGLE_REENABLE_DESTRUCTOR_OVERRIDE_WARNING
#endif

#if defined(__clang__)
#    define ANGLE_DISABLE_UNUSED_FUNCTION_WARNING \
        _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wunused-function\"")
#    define ANGLE_REENABLE_UNUSED_FUNCTION_WARNING _Pragma("clang diagnostic pop")
#else
#    define ANGLE_DISABLE_UNUSED_FUNCTION_WARNING
#    define ANGLE_REENABLE_UNUSED_FUNCTION_WARNING
#endif

#endif  // COMMON_DEBUG_H_
