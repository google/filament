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

#ifndef TNT_UTILS_OSTREAM_H
#define TNT_UTILS_OSTREAM_H

#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/PrivateImplementation.h>

#include <string>
#include <string_view>
#include <utility>

namespace utils::io {

struct ostream_;

class UTILS_PUBLIC  ostream : protected utils::PrivateImplementation<ostream_> {
    friend struct ostream_;

public:
    virtual ~ostream();

    ostream& operator<<(short value) noexcept;
    ostream& operator<<(unsigned short value) noexcept;

    ostream& operator<<(char value) noexcept;
    ostream& operator<<(unsigned char value) noexcept;

    ostream& operator<<(int value) noexcept;
    ostream& operator<<(unsigned int value) noexcept;

    ostream& operator<<(long value) noexcept;
    ostream& operator<<(unsigned long value) noexcept;

    ostream& operator<<(long long value) noexcept;
    ostream& operator<<(unsigned long long value) noexcept;

    ostream& operator<<(float value) noexcept;
    ostream& operator<<(double value) noexcept;
    ostream& operator<<(long double value) noexcept;

    ostream& operator<<(bool value) noexcept;

    ostream& operator<<(const void* value) noexcept;

    ostream& operator<<(const char* string) noexcept;
    ostream& operator<<(const unsigned char* string) noexcept;

    ostream& operator<<(std::string const& s) noexcept;
    ostream& operator<<(std::string_view const& s) noexcept;

    ostream& operator<<(ostream& (* f)(ostream&)) noexcept { return f(*this); }

    ostream& dec() noexcept;
    ostream& hex() noexcept;

protected:
    ostream& print(const char* format, ...) noexcept;

    class Buffer {
    public:
        Buffer() noexcept;
        ~Buffer() noexcept;

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        const char* get() const noexcept { return buffer; }

        std::pair<char*, size_t> grow(size_t s) noexcept;
        void advance(ssize_t n) noexcept;
        void reset() noexcept;

    private:
        void reserve(size_t newSize) noexcept;

        char* buffer = nullptr;     // buffer address
        char* curr = nullptr;       // current pointer
        size_t size = 0;            // size remaining
        size_t capacity = 0;        // total capacity of the buffer
    };

    Buffer& getBuffer() noexcept;
    Buffer const& getBuffer() const noexcept;

private:
    virtual ostream& flush() noexcept = 0;

    friend ostream& hex(ostream& s) noexcept;
    friend ostream& dec(ostream& s) noexcept;
    friend ostream& endl(ostream& s) noexcept;
    friend ostream& flush(ostream& s) noexcept;

    enum type {
        SHORT, USHORT, CHAR, UCHAR, INT, UINT, LONG, ULONG, LONG_LONG, ULONG_LONG, FLOAT, DOUBLE,
        LONG_DOUBLE
    };

    const char* getFormat(type t) const noexcept;
};

// handles utils::bitset
inline ostream& operator << (ostream& o, utils::bitset32 const& s) noexcept {
    return o << (void*)uintptr_t(s.getValue());
}

// handles vectors from libmath (but we do this generically, without needing a dependency on libmath)
template<template<typename T> class VECTOR, typename T>
inline ostream& operator<<(ostream& stream, const VECTOR<T>& v) {
    stream << "< ";
    for (size_t i = 0; i < v.size() - 1; i++) {
        stream << v[i] << ", ";
    }
    stream << v[v.size() - 1] << " >";
    return stream;
}

inline ostream& hex(ostream& s) noexcept { return s.hex(); }
inline ostream& dec(ostream& s) noexcept { return s.dec(); }
inline ostream& endl(ostream& s) noexcept { s << '\n'; return s.flush(); }
inline ostream& flush(ostream& s) noexcept { return s.flush(); }

} // namespace utils::io

#endif // TNT_UTILS_OSTREAM_H
