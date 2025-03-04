// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/common/Log.h"

#include <cstdio>
#include <string>

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/log.h>
#endif
#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#endif

namespace dawn {

namespace {

#if !defined(DAWN_DISABLE_LOGGING)
const char* SeverityName(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::Debug:
            return "Debug";
        case LogSeverity::Info:
            return "Info";
        case LogSeverity::Warning:
            return "Warning";
        case LogSeverity::Error:
            return "Error";
        default:
            DAWN_UNREACHABLE();
            return "";
    }
}
#endif

#if DAWN_PLATFORM_IS(ANDROID)
android_LogPriority AndroidLogPriority(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::Debug:
            return ANDROID_LOG_INFO;
        case LogSeverity::Info:
            return ANDROID_LOG_INFO;
        case LogSeverity::Warning:
            return ANDROID_LOG_WARN;
        case LogSeverity::Error:
            return ANDROID_LOG_ERROR;
        default:
            DAWN_UNREACHABLE();
            return ANDROID_LOG_ERROR;
    }
}
#endif  // DAWN_PLATFORM_IS(ANDROID)

}  // anonymous namespace

LogMessage::LogMessage(LogSeverity severity) : mSeverity(severity) {}

LogMessage::LogMessage(LogMessage&& other) = default;

LogMessage& LogMessage::operator=(LogMessage&& other) = default;

#if defined(DAWN_DISABLE_LOGGING)
LogMessage::~LogMessage() {
    (void)mSeverity;
    // Don't print logs to make fuzzing more efficient. Implemented as
    // an early return to avoid warnings about unused member variables.
    return;
}
#else  // defined(DAWN_DISABLE_LOGGING)
LogMessage::~LogMessage() {
    std::string fullMessage = mStream.str();

    // If this message has been moved, its stream is empty.
    if (fullMessage.empty()) {
        return;
    }

    const char* severityName = SeverityName(mSeverity);

#if DAWN_PLATFORM_IS(ANDROID)
    android_LogPriority androidPriority = AndroidLogPriority(mSeverity);
    __android_log_print(androidPriority, "Dawn", "%s: %s\n", severityName, fullMessage.c_str());
#else  // DAWN_PLATFORM_IS(ANDROID)
    FILE* outputStream = stdout;
    if (mSeverity == LogSeverity::Warning || mSeverity == LogSeverity::Error) {
        outputStream = stderr;
    }

#if DAWN_PLATFORM_IS(WINDOWS) && defined(OFFICIAL_BUILD)
    // If we are in a sandboxed process on an official build, the stdout/stderr
    // handles are likely not set, so trying to log to them will crash.
    HANDLE handle;
    if (outputStream == stderr) {
        handle = GetStdHandle(STD_ERROR_HANDLE);
    } else {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }
#endif  // DAWN_PLATFORM_IS(WINDOWS) && defined(OFFICIAL_BUILD)

    // Note: we use fprintf because <iostream> includes static initializers.
    fprintf(outputStream, "%s: %s\n", severityName, fullMessage.c_str());
    fflush(outputStream);
#endif  // DAWN_PLATFORM_IS(ANDROID)
}
#endif  // defined(DAWN_DISABLE_LOGGING)

LogMessage DebugLog() {
    return LogMessage(LogSeverity::Debug);
}

LogMessage InfoLog() {
    return LogMessage(LogSeverity::Info);
}

LogMessage WarningLog() {
    return LogMessage(LogSeverity::Warning);
}

LogMessage ErrorLog() {
    return LogMessage(LogSeverity::Error);
}

LogMessage DebugLog(const char* file, const char* function, int line) {
    LogMessage message = DebugLog();
    message << file << ":" << line << "(" << function << ")";
    return message;
}

}  // namespace dawn
