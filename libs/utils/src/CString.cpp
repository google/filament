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

#include <utils/CString.h>

#include <utils/compiler.h>
#include <utils/ostream.h>

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>


namespace utils {

UTILS_NOINLINE
CString::CString(const char* cstr, size_t const length) {
    if (length && cstr) {

        Data* const p = static_cast<Data*>(std::malloc(sizeof(Data) + length + 1));
        p->length = size_type(length);
        mCStr = reinterpret_cast<value_type*>(p + 1);
        // we don't use memcpy here to avoid a call to libc, the generated code is pretty good.
        std::uninitialized_copy_n(cstr, length, mCStr);
        mCStr[length] = '\0';
    }
}

CString::CString(size_t const length) {
    if (length) {
        Data* const p = static_cast<Data*>(std::malloc(sizeof(Data) + length + 1));
        p->length = size_type(length);
        mCStr = reinterpret_cast<value_type*>(p + 1);
        std::fill_n(mCStr, length, 0);
        mCStr[length] = '\0';
    }
}

CString::CString(const CString& rhs)
        : CString(rhs.c_str(), rhs.size()) {
}

CString& CString::operator=(const CString& rhs) {
    if (this != &rhs) {
        auto *const p = mData ? mData - 1 : nullptr;
        new(this) CString(rhs);
        std::free(p);
    }
    return *this;
}

CString::~CString() noexcept {
    if (mData) {
        std::free(mData - 1);
    }
}

CString& CString::replace(size_type const pos, size_type len, char const* str, size_t const l) & noexcept {
    assert(pos <= size());

    len = std::min(len, size() - pos);

    const size_type newSize = size() - len + l;

    // if the new string is not longer, we can do it in-place, which is much faster.
    if (newSize <= size()) {
        if (mCStr) {
            // This is equivalent to l <= len because capacity() == size()
            // move the tail of the string. +1 to move the null-terminator
            std::copy(mCStr + pos + len, mCStr + size() + 1, mCStr + pos + l);
            // copy the new content
            std::copy_n(str, l, mCStr + pos);
            // update the size
            (mData - 1)->length = newSize;
        }
        // if mCStr is null, newSize<=size() implies l<=len and since size is 0, len is 0, so l is 0.
        // so we're replacing nothing with nothing and there is nothing to do.
        return *this;
    }

    // Allocate enough memory to hold the new string.
    Data* const p = static_cast<Data*>(std::malloc(sizeof(Data) + newSize + 1));
    assert(p);
    p->length = newSize;
    value_type* newStr = reinterpret_cast<value_type*>(p + 1);

    const value_type* beginning = mCStr;
    const value_type* replacementStart = mCStr + pos;
    const value_type* replacementEnd = replacementStart + len;
    const value_type* end = beginning + size();

    value_type* ptr = newStr;
    ptr = std::uninitialized_copy(beginning, replacementStart, ptr);
    ptr = std::uninitialized_copy_n(str, l, ptr);
    ptr = std::uninitialized_copy(replacementEnd, end, ptr);

    // null-terminator
    *ptr = 0;

    std::swap(mCStr, newStr);
    if (newStr) {
        std::free(reinterpret_cast<Data*>(newStr) - 1);
    }

    return *this;
}

#if !defined(NDEBUG)
io::ostream& operator<<(io::ostream& out, const CString& rhs) {
    return out << rhs.c_str_safe();
}
#endif

namespace {

// use a C-style variadic function to avoid code bloat from templates
UTILS_NOINLINE
CString to_string_impl(const char* format, ...) noexcept {
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    int const len = vsnprintf(nullptr, 0, format, args1);
    va_end(args1);
    if (len >= 0) {
        CString s(len);
        vsnprintf(s.data(), len + 1, format, args2);
        va_end(args2);
        return s;
    }
    va_end(args2);
    return {};
}

} // anonymous namespace

template<>
CString to_string<int>(int const value) noexcept {
    return to_string_impl("%d", value);
}

template<>
CString to_string<unsigned int>(unsigned int const value) noexcept {
    return to_string_impl("%u", value);
}

template<>
CString to_string<short>(short const value) noexcept {
    return to_string_impl("%hd", value);
}

template<>
CString to_string<unsigned short>(unsigned short const value) noexcept {
    return to_string_impl("%hu", value);
}

template<>
CString to_string<long>(long const value) noexcept {
    return to_string_impl("%ld", value);
}

template<>
CString to_string<unsigned long>(unsigned long const value) noexcept {
    return to_string_impl("%lu", value);
}

template<>
CString to_string<long long>(long long const value) noexcept {
    return to_string_impl("%lld", value);
}

template<>
CString to_string<unsigned long long>(unsigned long long const value) noexcept {
    return to_string_impl("%llu", value);
}


template<>
CString to_string<float>(float const value) noexcept {
    return to_string_impl("%f", value);
}

} // namespace utils
