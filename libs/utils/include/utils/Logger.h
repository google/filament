/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_UTILS_LOGGER_H
#define TNT_UTILS_LOGGER_H

// Logger.h provides a subset of the Abseil logging API, offering the following macros:

// **LOG(severity)**: Logs a message at the specified severity level.
// **DLOG(severity)**: Logs a message at the specified severity level only in debug builds.

// Supported `severity` levels are:
// * `INFO`
// * `WARNING`
// * `ERROR`
// * `FATAL`

// For programmatic control over logging severity, use the `LEVEL` macro:

// **LOG(LEVEL(expression))**: Logs a message at a severity level determined by the `expression`.
// The `expression` must return a `utils::LogSeverity` value, which is equivalent to
// `absl::LogSeverity`.

#if defined(FILAMENT_USE_ABSEIL_LOGGING)

#include <absl/base/log_severity.h>
#include <absl/log/log.h>

namespace utils {
using absl::LogSeverity;
}

#else

#include <utils/Log.h>

namespace utils {

enum class LogSeverity : int {
    kInfo = 0,
    kWarning = 1,
    kError = 2,
    kFatal = 3,
};

template<typename Stream>
class LogLine {
public:
    explicit LogLine(Stream& stream)
        : mStream(stream) {}

    LogLine(const LogLine&) = delete;
    LogLine(LogLine&&) = delete;
    LogLine& operator=(const LogLine&) = delete;
    LogLine& operator=(LogLine&&) = delete;

    ~LogLine() noexcept { mStream << utils::io::endl; }

    template<typename T>
    LogLine& operator<<(T&& value) {
        mStream << std::forward<T>(value);
        return *this;
    }

private:
    Stream& mStream;
};

static inline io::ostream& getLogStream(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::kInfo:
            return slog.d;
        case LogSeverity::kWarning:
            return slog.w;
        case LogSeverity::kError:
            return slog.e;
        case LogSeverity::kFatal:
            return slog.e;
        default:
            return slog.d;
    }
}

struct NoopStream final {
    template<typename T>
    NoopStream& operator<<(const T&) {
        return *this;
    }
};

#define LOG(severity) LOG_IMPL_##severity

#ifndef NDEBUG
#define DLOG(severity) DLOG_IMPL_##severity
#else
#define DLOG(severity) utils::NoopStream()
#endif

#define DLOG_IMPL_INFO utils::LogLine(utils::slog.d)
#define DLOG_IMPL_WARNING utils::LogLine(utils::slog.d)
#define DLOG_IMPL_ERROR utils::LogLine(utils::slog.d)
#define DLOG_IMPL_LEVEL(severity) utils::LogLine(((void) severity, utils::slog.d))

#define LOG_IMPL_INFO utils::LogLine(utils::slog.i)
#define LOG_IMPL_WARNING utils::LogLine(utils::slog.w)
#define LOG_IMPL_ERROR utils::LogLine(utils::slog.e)
#define LOG_IMPL_LEVEL(severity) utils::LogLine(getLogStream(severity))

}// namespace utils
#endif

#endif// TNT_UTILS_LOGGER_H
