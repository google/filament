/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <utils/ostream.h>

#include "ostream_.h"

#include <utils/compiler.h>

#define UTILS_PRIVATE_IMPLEMENTATION_NON_COPYABLE
#include <utils/PrivateImplementation-impl.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

template class utils::PrivateImplementation<utils::io::ostream_>;

namespace utils::io {

ostream::~ostream() = default;

void ostream::setConsumer(ConsumerCallback consumer, void* user) noexcept {
    auto* const pImpl = mImpl;
    std::lock_guard const lock(pImpl->mLock);
    pImpl->mConsumer = { consumer, user };
}

ostream& flush(ostream& s) noexcept {
    auto* const pImpl = s.mImpl;
    pImpl->mLock.lock();
    auto const callback = pImpl->mConsumer;
    if (UTILS_UNLIKELY(callback.first)) {
        auto& buf = s.getBuffer();
        char const* const data = buf.get();
        if (UTILS_LIKELY(data)) {
            char* const str = strdup(data);
            buf.reset();
            pImpl->mLock.unlock();
            // call ConsumerCallback without lock held
            callback.first(callback.second, str);
            ::free(str);
            return s;
        }
    }
    pImpl->mLock.unlock();

    return s.flush();
}

ostream::Buffer& ostream::getBuffer() noexcept {
    return mImpl->mData;
}

ostream::Buffer const& ostream::getBuffer() const noexcept {
    return mImpl->mData;
}

const char* ostream::getFormat(ostream::type t) const noexcept {
    switch (t) {
        case type::SHORT:       return mImpl->mShowHex ? "0x%hx"  : "%hd";
        case type::USHORT:      return mImpl->mShowHex ? "0x%hx"  : "%hu";
        case type::CHAR:        return "%c";
        case type::UCHAR:       return "%c";
        case type::INT:         return mImpl->mShowHex ? "0x%x"   : "%d";
        case type::UINT:        return mImpl->mShowHex ? "0x%x"   : "%u";
        case type::LONG:        return mImpl->mShowHex ? "0x%lx"  : "%ld";
        case type::ULONG:       return mImpl->mShowHex ? "0x%lx"  : "%lu";
        case type::LONG_LONG:   return mImpl->mShowHex ? "0x%llx" : "%lld";
        case type::ULONG_LONG:  return mImpl->mShowHex ? "0x%llx" : "%llu";
        case type::FLOAT:       return "%.9g";
        case type::DOUBLE:      return "%.17g";
        case type::LONG_DOUBLE: return "%Lf";
    }
}

UTILS_NOINLINE
ostream& ostream::print(const char* format, ...) noexcept {
    va_list args0;
    va_list args1;

    // figure out how much size to we need
    va_start(args0, format);
    va_copy(args1, args0);
    ssize_t const s = vsnprintf(nullptr, 0, format, args0);
    va_end(args0);


    { // scope for the lock
        std::lock_guard const lock(mImpl->mLock);

        Buffer& buf = getBuffer();

        // grow the buffer to the needed size
        auto[curr, size] = buf.grow(s + 1); // +1 to include the null-terminator

        // print into the buffer
        vsnprintf(curr, size, format, args1);

        // advance the buffer
        buf.advance(s);
    }

    va_end(args1);

    return *this;
}

ostream& ostream::operator<<(short value) noexcept {
    const char* format = getFormat(type::SHORT);
    return print(format, value);
}

ostream& ostream::operator<<(unsigned short value) noexcept {
    const char* format = getFormat(type::USHORT);
    return print(format, value);
}

ostream& ostream::operator<<(char value) noexcept {
    const char* format = getFormat(type::CHAR);
    return print(format, value);
}

ostream& ostream::operator<<(unsigned char value) noexcept {
    const char* format = getFormat(type::UCHAR);
    return print(format, value);
}

ostream& ostream::operator<<(int value) noexcept {
    const char* format = getFormat(type::INT);
    return print(format, value);
}

ostream& ostream::operator<<(unsigned int value) noexcept {
    const char* format = getFormat(type::UINT);
    return print(format, value);
}

ostream& ostream::operator<<(long value) noexcept {
    const char* format = getFormat(type::LONG);
    return print(format, value);
}

ostream& ostream::operator<<(unsigned long value) noexcept {
    const char* format = getFormat(type::ULONG);
    return print(format, value);
}

ostream& ostream::operator<<(long long value) noexcept {
    const char* format = getFormat(type::LONG_LONG);
    return print(format, value);
}

ostream& ostream::operator<<(unsigned long long value) noexcept {
    const char* format = getFormat(type::ULONG_LONG);
    return print(format, value);
}

ostream& ostream::operator<<(float value) noexcept {
    const char* format = getFormat(type::FLOAT);
    return print(format, value);
}

ostream& ostream::operator<<(double value) noexcept {
    const char* format = getFormat(type::DOUBLE);
    return print(format, value);
}

ostream& ostream::operator<<(long double value) noexcept {
    const char* format = getFormat(type::LONG_DOUBLE);
    return print(format, value);
}

ostream& ostream::operator<<(bool value) noexcept {
    return operator<<(value ? "true" : "false");
}

ostream& ostream::operator<<(const char* string) noexcept {
    return print("%s", string);
}

ostream& ostream::operator<<(const unsigned char* string) noexcept {
    return print("%s", string);
}

ostream& ostream::operator<<(const void* value) noexcept {
    return print("%p", value);
}

ostream& ostream::operator<<(std::string const& s) noexcept {
    return print("%s", s.c_str());
}

ostream& ostream::operator<<(std::string_view const& s) noexcept {
    return print("%.*s", s.length(), s.data());
}

ostream& ostream::hex() noexcept {
    mImpl->mShowHex = true;
    return *this;
}

ostream& ostream::dec() noexcept {
    mImpl->mShowHex = false;
    return *this;
}

// ------------------------------------------------------------------------------------------------

// don't allocate any memory before we actually use the log because one of these is created
// per thread.
ostream::Buffer::Buffer() noexcept = default;

ostream::Buffer::~Buffer() noexcept {
    // note: on Android pre r14, thread_local destructors are not called
    free(buffer);
}

void ostream::Buffer::advance(ssize_t n) noexcept {
    if (n > 0) {
        size_t const written = n < size ? size_t(n) : size;
        curr += written;
        size -= written;
    }
}

void ostream::Buffer::reserve(size_t newSize) noexcept {
    size_t const offset = curr - buffer;
    if (buffer == nullptr) {
        buffer = (char*)malloc(newSize);
    } else {
        buffer = (char*)realloc(buffer, newSize);
    }
    assert(buffer);
    capacity = newSize;
    curr = buffer + offset;
    size = capacity - offset;
}

void ostream::Buffer::reset() noexcept {
    // aggressively shrink the buffer
    if (capacity > 1024) {
        free(buffer);
        buffer = (char*)malloc(1024);
        capacity = 1024;
    }
    curr = buffer;
    size = capacity;
}

size_t ostream::Buffer::length() const noexcept {
    return curr - buffer;
}

std::pair<char*, size_t> ostream::Buffer::grow(size_t s) noexcept {
    if (UTILS_UNLIKELY(size < s)) {
        size_t const used = curr - buffer;
        size_t const newCapacity = std::max(size_t(32), used + (s * 3 + 1) / 2); // 32 bytes minimum
        reserve(newCapacity);
        assert(size >= s);
    }
    return { curr, size };
}

} // namespace utils::io
