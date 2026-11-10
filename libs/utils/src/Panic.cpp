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

#include "ostream_.h"

#include <utils/CallStack.h>
#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/Log.h>
#include <utils/Logger.h>
#include <utils/Mutex.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/sstream.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif
#if defined(__ANDROID__)
#include <android/set_abort_message.h>
#endif

#include <cstdlib>
#include <cstring>
#include <mutex>
#include <new>
#include <string_view>
#include <utility>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__ANDROID__)
extern "C" __attribute__((weak)) void android_set_abort_message(const char *);
#endif

namespace utils {

// ------------------------------------------------------------------------------------------------

class UserPanicHandler {
    struct CallBack {
        Panic::PanicHandlerCallback handler = nullptr;
        void* user = nullptr;
        void call(Panic const& panic) const {
            if (UTILS_UNLIKELY(handler)) {
                handler(user, panic);
            }
        }
    };

    mutable Mutex mLock{};
    CallBack mCallBack{};

    CallBack getCallback() const noexcept {
        LockGuard const lock(mLock);
        return mCallBack;
    }

public:
    static UserPanicHandler& get() noexcept {
        static UserPanicHandler data;
        return data;
    }

    void call(Panic const& panic) const {
        getCallback().call(panic);
    }

    void set(Panic::PanicHandlerCallback const handler, void* user) noexcept {
        LockGuard const lock(mLock);
        mCallBack = { handler, user };
    }
};

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
static CString sprintfToString(const char* format, va_list args) noexcept {
    CString s;
    va_list tmp;
    va_copy(tmp, args);
    int n = vsnprintf(nullptr, 0, format, tmp);
    va_end(tmp);

    if (n >= 0) {
        ++n; // for the nul-terminating char
        char* const buf = new(std::nothrow) char[n];
        if (buf) {
            vsnprintf(buf, size_t(n), format, args);
            s = CString(buf);
            delete [] buf;
        }
    }
    return s;
}

static CString sprintfToString(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    CString const s{ sprintfToString(format, args) };
    va_end(args);
    return s;
}

static CString buildPanicString(
        std::string_view const& msg, const char* function, int const line,
        const char* file, const char* reason) {
#ifndef NDEBUG
    return sprintfToString("%.*s\nin %s:%d\nin file %s\nreason: %s",
            int(msg.size()), msg.data(), function, line, file, reason);
#else
    return sprintfToString("%.*s\nin %s:%d\nreason: %s",
            int(msg.size()), msg.data(), function, line, reason);
#endif
}

UTILS_ALWAYS_INLINE
static const char* formatFile(char const* file) noexcept {
    const char * p = std::strstr(file, "filament/");
    return p ? p : file;
}

// ------------------------------------------------------------------------------------------------

Panic::Panic(char const* type, char const* function, char const* file, int const line,
        char const* literal, CString reason)
        : mType(type),
          mFile(formatFile(file)),
          mFunction(function),
          mLine(line),
          mLiteral(literal),
          mReason(reason.empty() ? CString(literal) : std::move(reason)) {
    mCallstack.update(2);
    mWhat = buildPanicString(mType, mFunction, mLine, mFile, mReason.c_str());
}

Panic::~Panic() noexcept = default;

void Panic::setPanicHandler(PanicHandlerCallback const handler, void* user) noexcept {
    UserPanicHandler::get().set(handler, user);
}

const char* Panic::what() const noexcept {
    return mWhat.c_str();
}

const char* Panic::getType() const noexcept {
    return mType;
}

const char* Panic::getReason() const noexcept {
    return mReason.c_str();
}

const char* Panic::getReasonLiteral() const noexcept {
    return mLiteral.c_str();
}

const char* Panic::getFunction() const noexcept {
    return mFunction;
}

const char* Panic::getFile() const noexcept {
    return mFile;
}

int Panic::getLine() const noexcept {
    return mLine;
}

const CallStack& Panic::getCallStack() const noexcept {
    return mCallstack;
}

void Panic::log() const noexcept {
    LOG(ERROR) << what();
    LOG(ERROR) << mCallstack;
}

void Panic::reportToHandler(Panic const& e) {
#ifndef __EXCEPTIONS
    // log the Panic at the point it is detected unless we have exceptions, the exception handler will be
    // responsible for that.
    e.log();
#endif
    UserPanicHandler::get().call(e);
}

void Panic::abortWithMessage(Panic const& e) {
    // Register the full panic message as the tombstone "Abort message:" so that it
    // appears in crash reports collected by Google Play Console and other tools that
    // read tombstones from field devices (which never have access to logcat output).
#if defined(__ANDROID__)
    if (__builtin_available(android 30, *)) {
        __android_log_default_aborter(e.what());
    } else {
        // For API < 30, we can try to use this private API if it's available.
        if (&android_set_abort_message) {
            android_set_abort_message(e.what());
        }
    }
#endif
    // and finally abort if we somehow get here
    std::abort();
}

// ------------------------------------------------------------------------------------------------

template<typename T>
TPanic<T>::TPanic(const char* function, const char* file, int const line, char const* literal,
        CString reason)
        : Panic(T::type, function, file, line, literal, std::move(reason)) {
}

template<typename T>
TPanic<T>::~TPanic() noexcept = default;

template<typename T>
void TPanic<T>::panic(char const* function, char const* file, int const line, char const* literal,
        const char* format, ...) {
    va_list args;
    va_start(args, format);
    CString reason{ sprintfToString(format, args) };
    va_end(args);

    panic(function, file, line, literal, std::move(reason));
}

template<typename T>
void TPanic<T>::panic(char const* function, char const* file, int line, char const* literal,
        CString reason) {
    T e(function, file, line, literal, std::move(reason));
    reportToHandler(e);
#ifdef __EXCEPTIONS
    throw std::move(e);
#endif
    abortWithMessage(e);
}

// -----------------------------------------------------------------------------------------------

namespace details {

void panicLog(char const* function, char const* file, int const line, const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    CString const reason{ sprintfToString(format, args) };
    va_end(args);

    CString const msg = buildPanicString("PanicLog", function, line, file, reason.c_str());
    LOG(ERROR) << msg;
    LOG(ERROR) << CallStack::unwind(1);
}

struct PanicStream::Impl {
    io::sstream stream;
    char const* function;
    char const* file;
    int line;
    char const* literal;
};

PanicStream::PanicStream(
        char const* function,
        char const* file,
        int const line,
        char const* message) noexcept
    : mImpl(new(std::nothrow) Impl{ {}, function, file, line, message }) {
}

PanicStream::~PanicStream() noexcept {
    delete mImpl;
}

PanicStream& PanicStream::operator<<(short const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned short const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(char const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned char const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(int const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned int const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(long const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned long const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(long long int const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned long long int const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(float const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(double const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(long double const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(bool const value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(void const* value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(char const* value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(unsigned char const* value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(CString const& value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

PanicStream& PanicStream::operator<<(std::string_view const& value) noexcept {
    if (UTILS_LIKELY(mImpl)) mImpl->stream << value;
    return *this;
}

void PanicStream::logWarning() noexcept {
    if (UTILS_LIKELY(mImpl)) {
        LOG(WARNING) << "Failed [" << mImpl->literal << "]" << "\n  file=" << mImpl->file
                     << "\n  function=" << mImpl->function << "\n  line=" << mImpl->line << "\n  info=\""
                     << mImpl->stream.c_str() << "\"";
    }
}

template<typename T>
TPanicStream<T>::~TPanicStream() noexcept(false) {
    if (UTILS_LIKELY(mImpl)) {
        T::panic(mImpl->function, mImpl->file, mImpl->line, mImpl->literal,
                CString(mImpl->stream.c_str()));
    } else {
        T::panic("", "", 0, "", CString());
    }
}

template<typename PanicType>
FlagGuardedStream<PanicType>::FlagGuardedStream(bool const enable, char const* function,
        char const* file, int const line, char const* condition) noexcept
    : PanicStream(function, file, line, condition),
      mEnablePanic(enable) {
}

template<typename PanicType>
FlagGuardedStream<PanicType>::~FlagGuardedStream() noexcept(false) {
    if (mEnablePanic) {
        if (UTILS_LIKELY(mImpl)) {
            PanicType::panic(mImpl->function, mImpl->file, mImpl->line, mImpl->literal,
                    CString(mImpl->stream.c_str()));
        } else {
            PanicType::panic("", "", 0, "", CString());
        }
    } else {
        logWarning();
    }
}

template class UTILS_PUBLIC TPanicStream<PreconditionPanic>;
template class UTILS_PUBLIC TPanicStream<PostconditionPanic>;
template class UTILS_PUBLIC TPanicStream<ArithmeticPanic>;

template class UTILS_PUBLIC FlagGuardedStream<PreconditionPanic>;
template class UTILS_PUBLIC FlagGuardedStream<PostconditionPanic>;
template class UTILS_PUBLIC FlagGuardedStream<ArithmeticPanic>;

} // namespace details

// -----------------------------------------------------------------------------------------------

template class UTILS_PUBLIC TPanic<PreconditionPanic>;
template class UTILS_PUBLIC TPanic<PostconditionPanic>;
template class UTILS_PUBLIC TPanic<ArithmeticPanic>;

} // namespace utils
