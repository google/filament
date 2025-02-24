//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"

#include <stdarg.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ostream>
#include <vector>

#if defined(ANGLE_PLATFORM_ANDROID)
#    include <android/log.h>
#endif

#if defined(ANGLE_PLATFORM_APPLE)
#    include <os/log.h>
#endif

#if defined(ANGLE_PLATFORM_WINDOWS)
#    include <windows.h>
#endif

#include "anglebase/no_destructor.h"
#include "common/Optional.h"
#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "common/entry_points_enum_autogen.h"
#include "common/system_utils.h"

namespace gl
{

namespace
{

DebugAnnotator *g_debugAnnotator = nullptr;

angle::SimpleMutex *g_debugMutex = nullptr;

constexpr std::array<const char *, LOG_NUM_SEVERITIES> g_logSeverityNames = {
    {"EVENT", "INFO", "WARN", "ERR", "FATAL"}};

constexpr const char *LogSeverityName(int severity)
{
    return (severity >= 0 && severity < LOG_NUM_SEVERITIES) ? g_logSeverityNames[severity]
                                                            : "UNKNOWN";
}

bool ShouldCreateLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#elif defined(ANGLE_ALWAYS_LOG_INFO)
    return severity == LOG_FATAL || severity == LOG_ERR || severity == LOG_WARN ||
           severity == LOG_INFO;
#elif defined(ANGLE_ENABLE_ASSERTS)
    return severity == LOG_FATAL || severity == LOG_ERR || severity == LOG_WARN;
#else
    return severity == LOG_FATAL || severity == LOG_ERR;
#endif
}

}  // namespace

namespace priv
{

bool ShouldCreatePlatformLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#else
    return severity != LOG_EVENT;
#endif
}

// This is never instantiated, it's just used for EAT_STREAM_PARAMETERS to an object of the correct
// type on the LHS of the unused part of the ternary operator.
std::ostream *gSwallowStream;
}  // namespace priv

bool DebugAnnotationsActive(const gl::Context *context)
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS) || defined(ANGLE_ENABLE_DEBUG_TRACE)
    return g_debugAnnotator != nullptr && g_debugAnnotator->getStatus(context);
#else
    return false;
#endif
}

bool ShouldBeginScopedEvent(const gl::Context *context)
{
#if defined(ANGLE_ENABLE_ANNOTATOR_RUN_TIME_CHECKS)
    return DebugAnnotationsActive(context);
#else
    return true;
#endif  // defined(ANGLE_ENABLE_ANNOTATOR_RUN_TIME_CHECKS)
}

bool DebugAnnotationsInitialized()
{
    return g_debugAnnotator != nullptr;
}

void InitializeDebugAnnotations(DebugAnnotator *debugAnnotator)
{
    UninitializeDebugAnnotations();
    g_debugAnnotator = debugAnnotator;
}

void UninitializeDebugAnnotations()
{
    // Pointer is not managed.
    g_debugAnnotator = nullptr;
}

void InitializeDebugMutexIfNeeded()
{
    if (g_debugMutex == nullptr)
    {
        g_debugMutex = new angle::SimpleMutex();
    }
}

angle::SimpleMutex &GetDebugMutex()
{
    ASSERT(g_debugMutex);
    return *g_debugMutex;
}

ScopedPerfEventHelper::ScopedPerfEventHelper(gl::Context *context, angle::EntryPoint entryPoint)
    : mContext(context), mEntryPoint(entryPoint), mFunctionName(nullptr), mCalledBeginEvent(false)
{}

ScopedPerfEventHelper::~ScopedPerfEventHelper()
{
    // EGL_Initialize() and EGL_Terminate() can change g_debugAnnotator.  Must check the value of
    // g_debugAnnotator and whether ScopedPerfEventHelper::begin() initiated a begine that must be
    // ended now.
    if (DebugAnnotationsInitialized() && mCalledBeginEvent)
    {
        g_debugAnnotator->endEvent(mContext, mFunctionName, mEntryPoint);
    }
}

void ScopedPerfEventHelper::begin(const char *format, ...)
{
    mFunctionName = GetEntryPointName(mEntryPoint);

    va_list vararg;
    va_start(vararg, format);

    std::vector<char> buffer;
    size_t len = FormatStringIntoVector(format, vararg, buffer);
    va_end(vararg);

    ANGLE_LOG(EVENT) << std::string(&buffer[0], len);
    if (DebugAnnotationsInitialized())
    {
        mCalledBeginEvent = true;
        g_debugAnnotator->beginEvent(mContext, mEntryPoint, mFunctionName, buffer.data());
    }
}

LogMessage::LogMessage(const char *file, const char *function, int line, LogSeverity severity)
    : mFile(file), mFunction(function), mLine(line), mSeverity(severity)
{
    // INFO() and EVENT() do not require additional function(line) info.
    if (mSeverity > LOG_INFO)
    {
        const char *slash = std::max(strrchr(mFile, '/'), strrchr(mFile, '\\'));
        mStream << (slash ? (slash + 1) : mFile) << ":" << mLine << " (" << mFunction << "): ";
    }
}

LogMessage::~LogMessage()
{
    {
        std::unique_lock<angle::SimpleMutex> lock;
        if (g_debugMutex != nullptr)
        {
            lock = std::unique_lock<angle::SimpleMutex>(*g_debugMutex);
        }

        if (DebugAnnotationsInitialized() && (mSeverity > LOG_INFO))
        {
            g_debugAnnotator->logMessage(*this);
        }
        else
        {
            Trace(getSeverity(), getMessage().c_str());
        }
    }

    if (mSeverity == LOG_FATAL)
    {
        if (angle::IsDebuggerAttached())
        {
            angle::BreakDebugger();
        }
        else
        {
            ANGLE_CRASH();
        }
    }
}

void Trace(LogSeverity severity, const char *message)
{
    if (!ShouldCreateLogMessage(severity))
    {
        return;
    }

    std::string str(message);

    if (DebugAnnotationsActive(/*context=*/nullptr))
    {

        switch (severity)
        {
            case LOG_EVENT:
                // Debugging logging done in ScopedPerfEventHelper
                break;
            default:
                g_debugAnnotator->setMarker(/*context=*/nullptr, message);
                break;
        }
    }

    if (severity == LOG_FATAL || severity == LOG_ERR || severity == LOG_WARN ||
#if defined(ANGLE_ENABLE_TRACE_ANDROID_LOGCAT) || defined(ANGLE_ENABLE_TRACE_EVENTS)
        severity == LOG_EVENT ||
#endif
        severity == LOG_INFO)
    {
#if defined(ANGLE_PLATFORM_ANDROID)
        android_LogPriority android_priority = ANDROID_LOG_ERROR;
        switch (severity)
        {
            case LOG_INFO:
            case LOG_EVENT:
                android_priority = ANDROID_LOG_INFO;
                break;
            case LOG_WARN:
                android_priority = ANDROID_LOG_WARN;
                break;
            case LOG_ERR:
                android_priority = ANDROID_LOG_ERROR;
                break;
            case LOG_FATAL:
                android_priority = ANDROID_LOG_FATAL;
                break;
            default:
                UNREACHABLE();
        }
        __android_log_print(android_priority, "ANGLE", "%s: %s\n", LogSeverityName(severity),
                            str.c_str());
        // Note: we also log to stdout/stderr below.
#endif

#if defined(ANGLE_PLATFORM_APPLE)
        if (__builtin_available(macOS 10.12, iOS 10.0, *))
        {
            os_log_type_t apple_log_type = OS_LOG_TYPE_DEFAULT;
            switch (severity)
            {
                case LOG_INFO:
                case LOG_EVENT:
                    apple_log_type = OS_LOG_TYPE_INFO;
                    break;
                case LOG_WARN:
                    apple_log_type = OS_LOG_TYPE_DEFAULT;
                    break;
                case LOG_ERR:
                    apple_log_type = OS_LOG_TYPE_ERROR;
                    break;
                case LOG_FATAL:
                    // OS_LOG_TYPE_FAULT is too severe - grabs the entire process tree.
                    apple_log_type = OS_LOG_TYPE_ERROR;
                    break;
                default:
                    UNREACHABLE();
            }
            os_log_with_type(OS_LOG_DEFAULT, apple_log_type, "ANGLE: %s: %s\n",
                             LogSeverityName(severity), str.c_str());
        }
#else
        // Note: we use fprintf because <iostream> includes static initializers.
        fprintf((severity >= LOG_WARN) ? stderr : stdout, "%s: %s\n", LogSeverityName(severity),
                str.c_str());
#endif
    }

#if defined(ANGLE_PLATFORM_WINDOWS) && \
    (defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER) || !defined(NDEBUG))
#    if !defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    if (severity >= LOG_ERR)
#    endif  // !defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    {
        OutputDebugStringA(str.c_str());
        OutputDebugStringA("\n");
    }
#endif

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#    if defined(NDEBUG)
    if (severity == LOG_EVENT || severity == LOG_WARN || severity == LOG_INFO)
    {
        return;
    }
#    endif  // defined(NDEBUG)
    static angle::base::NoDestructor<std::ofstream> file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file->good())
    {
        if (severity > LOG_EVENT)
        {
            *file << LogSeverityName(severity) << ": ";
        }
        *file << str << "\n";
        file->flush();
    }
#endif  // defined(ANGLE_ENABLE_DEBUG_TRACE)
}

LogSeverity LogMessage::getSeverity() const
{
    return mSeverity;
}

std::string LogMessage::getMessage() const
{
    return mStream.str();
}

#if defined(ANGLE_PLATFORM_WINDOWS)
priv::FmtHexHelper<HRESULT, char> FmtHR(HRESULT value)
{
    return priv::FmtHexHelper<HRESULT, char>("HRESULT: ", value);
}

priv::FmtHexHelper<DWORD, char> FmtErr(DWORD value)
{
    return priv::FmtHexHelper<DWORD, char>("error: ", value);
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

}  // namespace gl
