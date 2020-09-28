/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_UTILS_LOG_H
#define TNT_UTILS_LOG_H

#include <string>

#include <utils/ThreadLocal.h>
#include <utils/bitset.h>
#include <utils/compiler.h> // ssize_t is a POSIX type.

#include <utils/ostream.h>

namespace utils {
namespace io {

class UTILS_PUBLIC LogStream : public ostream {
public:

    enum Priority {
        LOG_DEBUG, LOG_ERROR, LOG_WARNING, LOG_INFO
    };

    explicit LogStream(Priority p) noexcept : mPriority(p) {}

    ostream& flush() noexcept override;

private:
    Priority mPriority;
};

} // namespace io

struct UTILS_PUBLIC Loggers {
    // DEBUG level logging stream
    io::LogStream& d;

    // ERROR level logging stream
    io::LogStream& e;

    // WARNING level logging stream
    io::LogStream& w;

    // INFORMATION level logging stream
    io::LogStream& i;
};

extern UTILS_PUBLIC const Loggers slog;

} // namespace utils

#endif // TNT_UTILS_LOG_H
