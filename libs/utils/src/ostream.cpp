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

#include <string>
#include <utils/compiler.h>

namespace utils {

namespace io {

ostream::~ostream() = default;

ostream::Buffer::Buffer() noexcept {
    constexpr size_t initialSize = 1024;
    buffer = (char*) malloc(initialSize);
    assert(buffer);
    // Set the first byte to 0 as this buffer might be used as a C string.
    buffer[0] = 0;
    curr = buffer;
    capacity = initialSize;
    size = initialSize;
}

ostream::Buffer::~Buffer() noexcept {
    free(buffer);
}

UTILS_NOINLINE
void ostream::Buffer::advance(ssize_t n) noexcept {
    if (n > 0) {
        size_t written = n < size ? size_t(n) : size;
        curr += written;
        size -= written;
    }
}

UTILS_NOINLINE
void ostream::Buffer::resize(size_t newSize) noexcept {
    size_t offset = curr - buffer;
    buffer = (char*) realloc(buffer, newSize);
    assert(buffer);
    capacity = newSize;
    curr = buffer + offset;
    size = capacity - offset;
}

UTILS_NOINLINE
void ostream::Buffer::reset() noexcept {
    curr = buffer;
    size = capacity;
}

const char* ostream::getFormat(ostream::type t) const noexcept {
    switch (t) {
        case type::SHORT:       return mShowHex ? "0x%hx"  : "%hd";
        case type::USHORT:      return mShowHex ? "0x%hx"  : "%hu";
        case type::CHAR:        return "%c";
        case type::UCHAR:       return "%c";
        case type::INT:         return mShowHex ? "0x%x"   : "%d";
        case type::UINT:        return mShowHex ? "0x%x"   : "%u";
        case type::LONG:        return mShowHex ? "0x%lx"  : "%ld";
        case type::ULONG:       return mShowHex ? "0x%lx"  : "%lu";
        case type::LONG_LONG:   return mShowHex ? "0x%llx" : "%lld";
        case type::ULONG_LONG:  return mShowHex ? "0x%llx" : "%llu";
        case type::DOUBLE:      return "%f";
        case type::LONG_DOUBLE: return "%Lf";
    }
}

void ostream::growBufferIfNeeded(size_t s) noexcept {
    Buffer& buf = getBuffer();
    const size_t used = buf.curr - buf.buffer;  // space currently used in buffer
    if (UTILS_UNLIKELY(buf.size < s)) {
        size_t newSize = buf.capacity * 2;
        while (UTILS_UNLIKELY((newSize - used) < s)) {
            newSize *= 2;
        }
        buf.resize(newSize);
        assert(buf.size >= s);
    }
}

ostream& ostream::operator<<(short value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::SHORT), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::SHORT), value));
    return *this;
}

ostream& ostream::operator<<(unsigned short value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::USHORT), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::USHORT), value));
    return *this;
}

ostream& ostream::operator<<(char value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::CHAR), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::CHAR), value));
    return *this;
}

ostream& ostream::operator<<(unsigned char value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::UCHAR), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::UCHAR), value));
    return *this;
}

ostream& ostream::operator<<(int value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::INT), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::INT), value));
    return *this;
}

ostream& ostream::operator<<(unsigned int value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::UINT), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::UINT), value));
    return *this;
}

ostream& ostream::operator<<(long value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::LONG), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::LONG), value));
    return *this;
}

ostream& ostream::operator<<(unsigned long value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::ULONG), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::ULONG), value));
    return *this;
}

ostream& ostream::operator<<(long long value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::LONG_LONG), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::LONG_LONG), value));
    return *this;
}

ostream& ostream::operator<<(unsigned long long value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::ULONG_LONG), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::ULONG_LONG), value));
    return *this;
}

ostream& ostream::operator<<(float value) noexcept {
    return operator<<((double)value);
}

ostream& ostream::operator<<(double value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::DOUBLE), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::DOUBLE), value));
    return *this;
}

ostream& ostream::operator<<(long double value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, getFormat(type::LONG_DOUBLE), value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, getFormat(type::LONG_DOUBLE), value));
    return *this;
}

ostream& ostream::operator<<(bool value) noexcept {
    return operator<<(value ? "true" : "false");
}

ostream& ostream::operator<<(const char* string) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, "%s", string);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, "%s", string));
    return *this;
}

ostream& ostream::operator<<(const unsigned char* string) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, "%s", string);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, "%s", string));
    return *this;
}

ostream& ostream::operator<<(const void* value) noexcept {
    Buffer& buf = getBuffer();
    size_t s = snprintf(nullptr, 0, "%p", value);
    growBufferIfNeeded(s + 1); // +1 to include the null-terminator
    buf.advance(snprintf(buf.curr, buf.size, "%p", value));
    return *this;
}

ostream& ostream::hex() noexcept {
    mShowHex = true;
    return *this;
}

ostream& ostream::dec() noexcept {
    mShowHex = false;
    return *this;
}

} // namespace io

} // namespace utils
