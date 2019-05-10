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

#include <utils/Log.h>

#include <string>
#include <utils/compiler.h>

#ifdef ANDROID
#   include <android/log.h>
#   ifndef UTILS_LOG_TAG
#       define UTILS_LOG_TAG "Filament"
#   endif
#endif

namespace utils {

namespace io {

ostream& LogStream::flush() noexcept {
    Buffer& buf = getBuffer();
#if ANDROID
    switch (mPriority) {
        case DEBUG:
            __android_log_write(ANDROID_LOG_DEBUG, UTILS_LOG_TAG, buf.get());
            break;
        case ERROR:
            __android_log_write(ANDROID_LOG_ERROR, UTILS_LOG_TAG, buf.get());
            break;
        case WARNING:
            __android_log_write(ANDROID_LOG_WARN, UTILS_LOG_TAG, buf.get());
            break;
        case INFO:
            __android_log_write(ANDROID_LOG_INFO, UTILS_LOG_TAG, buf.get());
            break;
    }
#else
    switch (mPriority) {
        case DEBUG:
        case WARNING:
        case INFO:
            fprintf(stdout, "%s", buf.get());
            break;
        case ERROR:
            fprintf(stderr, "%s", buf.get());
            break;
    }
#endif
    buf.reset();
    return *this;
}

static LogStream cout(LogStream::Priority::DEBUG);
static LogStream cerr(LogStream::Priority::ERROR);
static LogStream cwarn(LogStream::Priority::WARNING);
static LogStream cinfo(LogStream::Priority::INFO);

} // namespace io


Loggers slog = {
        io::cout,   // debug
        io::cerr,   // error
        io::cwarn,  // warning
        io::cinfo   // info
};

} // namespace utils
