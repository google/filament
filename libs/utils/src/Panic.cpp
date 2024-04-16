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

#include <utils/compiler.h>
#include <utils/Panic.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <cstdlib>
#include <cstring>
#include <mutex>
#include <new>
#include <string>
#include <utility>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace utils {

// ------------------------------------------------------------------------------------------------

class UserPanicHandler {
    struct CallBack {
        Panic::PanicHandlerCallback handler = nullptr;
        void* user = nullptr;
        void call(Panic const& panic) const noexcept {
            if (UTILS_UNLIKELY(handler)) {
                handler(user, panic);
            }
        }
    };

    mutable std::mutex mLock{};
    CallBack mCallBack{};

    CallBack getCallback() const noexcept {
        std::lock_guard const lock(mLock);
        return mCallBack;
    }

public:
    static UserPanicHandler& get() noexcept {
        static UserPanicHandler data;
        return data;
    }

    void call(Panic const& panic) const noexcept {
        getCallback().call(panic);
    }

    void set(Panic::PanicHandlerCallback handler, void* user) noexcept {
        std::lock_guard const lock(mLock);
        mCallBack = { handler, user };
    }
};

// ------------------------------------------------------------------------------------------------

static std::string formatString(const char* format, va_list args) noexcept {
    std::string reason;

    va_list tmp;
    va_copy(tmp, args);
    int n = vsnprintf(nullptr, 0, format, tmp);
    va_end(tmp);

    if (n >= 0) {
        ++n; // for the nul-terminating char
        char* const buf = new(std::nothrow) char[n];
        if (buf) {
            vsnprintf(buf, size_t(n), format, args);
            reason.assign(buf);
            delete [] buf;
        }
    }
    return reason;
}

static std::string formatString(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    std::string s(formatString(format, args));
    va_end(args);
    return s;
}

static std::string panicString(
        const std::string& msg, const char* function, int line,
        const char* file, const char* reason) {
#ifndef NDEBUG
    return formatString("%s\nin %s:%d\nin file %s\nreason: %s",
            msg.c_str(), function, line, file, reason);
#else
    return formatString("%s\nin %s:%d\nreason: %s",
            msg.c_str(), function, line, reason);
#endif
}

// ------------------------------------------------------------------------------------------------

Panic::~Panic() noexcept = default;

void Panic::setPanicHandler(PanicHandlerCallback handler, void* user) noexcept {
    UserPanicHandler::get().set(handler, user);
}

// ------------------------------------------------------------------------------------------------

template<typename T>
TPanic<T>::TPanic(std::string reason) :
    m_reason(std::move(reason)) {
    m_callstack.update(1);
    buildMessage();
}

template<typename T>
TPanic<T>::TPanic(const char* function, const char* file, int line, std::string reason)
        : m_reason(std::move(reason)), m_function(function), m_file(file), m_line(line) {
    m_callstack.update(1);
    buildMessage();
}

template<typename T>
TPanic<T>::~TPanic() {
}

template<typename T>
const char* TPanic<T>::what() const noexcept {
    return m_msg.c_str();
}

template<typename T>
const char* TPanic<T>::getReason() const noexcept {
    return m_reason.c_str();
}

template<typename T>
const char* TPanic<T>::getFunction() const noexcept {
    return m_function;
}

template<typename T>
const char* TPanic<T>::getFile() const noexcept {
    return m_file;
}

template<typename T>
int TPanic<T>::getLine() const noexcept {
    return m_line;
}

template<typename T>
const CallStack& TPanic<T>::getCallStack() const noexcept {
    return m_callstack;
}

template<typename T>
void TPanic<T>::log() const noexcept {
    slog.e << what() << io::endl;
    slog.e << m_callstack << io::endl;
}

template<typename T>
void TPanic<T>::buildMessage() {
    std::string type;
#if UTILS_HAS_RTTI
    type = CallStack::demangleTypeName(typeid(T).name()).c_str();
#else
    type = "Panic";
#endif
    m_msg = panicString(type, m_function, m_line, m_file, m_reason.c_str());
}

UTILS_ALWAYS_INLINE
inline static const char* formatFile(char const* file) noexcept {
    const char * p = std::strstr(file, "filament/");
    return p ? p : file;
}

template<typename T>
void TPanic<T>::panic(char const* function, char const* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string const reason(formatString(format, args));
    va_end(args);
    T e(function, formatFile(file), line, reason);

    // always log the Panic at the point it is detected
    e.log();

    // Call the user provided handler
    UserPanicHandler::get().call(e);

    // if exceptions are enabled, throw now.
#ifdef __EXCEPTIONS
    throw e;
#endif

    // and finally abort if we somehow get here
    std::abort();
}

// -----------------------------------------------------------------------------------------------

namespace details {

void panicLog(char const* function, char const* file, int line, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    std::string const reason(formatString(format, args));
    va_end(args);

    const std::string msg = panicString("" /* no extra message */,
            function, line, file, reason.c_str());

    slog.e << msg << io::endl;
    slog.e << CallStack::unwind(1) << io::endl;
}

} // namespace details

// -----------------------------------------------------------------------------------------------

template class UTILS_PUBLIC TPanic<PreconditionPanic>;
template class UTILS_PUBLIC TPanic<PostconditionPanic>;
template class UTILS_PUBLIC TPanic<ArithmeticPanic>;

} // namespace utils
