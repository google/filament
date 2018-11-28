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

// We always use TINY IO, it seems to work well enough and that a nice way to not
// break the android builds
#define UTILS_TINY_IO 1

#ifndef UTILS_TINY_IO
#   ifdef ANDROID
#       define UTILS_TINY_IO 1
#   else
#       define UTILS_TINY_IO 0
#   endif
#endif

#if !UTILS_TINY_IO
#include <ios>
#include <ostream>
#endif

namespace utils {
namespace io {

#if UTILS_TINY_IO

class ostream {
public:
    enum Priority {
        DEBUG, ERROR, WARNING, INFO
    };

    explicit ostream(Priority p) noexcept : mPriority(p) { }

    ostream& operator<<(short value) noexcept;
    ostream& operator<<(unsigned short value) noexcept;

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

    ostream& operator<<(ostream& (* f)(ostream&)) noexcept { return f(*this); }


private:
    ostream& dec() noexcept;
    ostream& hex() noexcept;
    ostream& flush() noexcept;

    friend ostream& hex(ostream& s) noexcept;
    friend ostream& dec(ostream& s) noexcept;
    friend ostream& endl(ostream& s) noexcept;
    friend ostream& flush(ostream& s) noexcept;

    enum type {
        SHORT, USHORT, INT, UINT, LONG, ULONG, LONG_LONG, ULONG_LONG, DOUBLE, LONG_DOUBLE
    };

    bool mShowHex = false;
    Priority mPriority;
    const char* getFormat(type t) const noexcept;

    class Buffer {
        char mStorage[1024];
    public:
        char* curr = mStorage;
        size_t size = sizeof(mStorage);
        const char* get() const noexcept { return mStorage; }
        void advance(ssize_t n) noexcept;
        void reset() noexcept;
    };

    Buffer& getBuffer() noexcept { return mData; }

    static UTILS_DECLARE_TLS(Buffer) mData;
};

// handles std::string
inline ostream& operator << (ostream& o, std::string const& s) noexcept { return o << s.c_str(); }

// handles utils::bitset
inline ostream& operator << (ostream& o, utils::bitset32 const& s) noexcept {
    return o << (void*)uintptr_t(s.getValue());
}

// handles vectors from libmath (but we do this generically, without needing a dependency on libmath)
template<template<typename T> class VECTOR, typename T>
inline ostream& operator<<(ostream& stream, const VECTOR<T>& v) {
    stream << "< ";
    for (size_t i = 0; i < v.size() - 1; i++) {
        stream << T(v[i]) << ", ";
    }
    stream << T(v[v.size() - 1]) << " >";
    return stream;
}

inline ostream& hex(ostream& s) noexcept { return s.hex(); }
inline ostream& dec(ostream& s) noexcept { return s.dec(); }
inline ostream& endl(ostream& s) noexcept { s << "\n"; return s.flush(); }
inline ostream& flush(ostream& s) noexcept { return s.flush(); }

#else  // UTILS_TINY_IO

using ostream = std::ostream;
inline ostream& hex(ostream& s) noexcept { return s << std::hex; }
inline ostream& dec(ostream& s) noexcept { return s << std::dec; }
inline ostream& endl(ostream& s) noexcept { return s << std::endl; }
inline ostream& flush(ostream& s) noexcept { return s << std::flush; }

#endif // UTILS_TINY_IO
} // namespace io

struct Loggers {
    // DEBUG level logging stream
    io::ostream& d;

    // ERROR level logging stream
    io::ostream& e;

    // WARNING level logging stream
    io::ostream& w;

    // INFORMATION level logging stream
    io::ostream& i;
};

extern Loggers slog;

} // namespace utils

#endif // TNT_UTILS_LOG_H
