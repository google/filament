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

#ifndef TNT_UTILS_CSTRING_H
#define TNT_UTILS_CSTRING_H

// NOTE: this header should not include STL headers

#include <utils/compiler.h>
#include <utils/StaticString.h>

#include <string_view>
#include <utility>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace utils {
namespace io {
class ostream;
}

//! \privatesection
struct hashCStrings {
    typedef const char* argument_type;
    typedef size_t result_type;
    result_type operator()(argument_type cstr) const noexcept {
        size_t hash = 5381;
        while (int const c = static_cast<unsigned char>(*cstr++)) {
            hash = (hash * 33u) ^ size_t(c);
        }
        return hash;
    }
};

template <size_t N>
using StringLiteral = const char[N];


// ------------------------------------------------------------------------------------------------

class UTILS_PUBLIC CString {
public:
    using value_type      = char;
    using size_type       = uint32_t;
    using difference_type = int32_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using iterator        = value_type*;
    using const_iterator  = const value_type*;

    CString() noexcept {} // NOLINT(modernize-use-equals-default), Ubuntu compiler bug

    // Allocates memory and appends a null. This constructor can be used to hold arbitrary data
    // inside the string (i.e. it can contain nulls or non-ASCII encodings).
    CString(const char* cstr, size_t length);

    // Allocates memory for a string of size length plus space for the null terminating character.
    // Also initializes the memory to 0. This constructor can be used to hold arbitrary data
    // inside the string.
    explicit CString(size_t length);

    // Allocates memory and copies traditional C string content. Unlike the above constructor, this
    // does not allow embedded nulls. This is explicit because this operation is costly.
    explicit CString(const char* cstr);

    template<size_t N>
    CString(StringLiteral<N> const& other) noexcept // NOLINT(google-explicit-constructor)
            : CString(other, N - 1) {
    }

    CString(StaticString const& other) noexcept
        : CString(other.c_str(), other.length()) {}

    CString(const CString& rhs);

    CString(CString&& rhs) noexcept {
        this->swap(rhs);
    }


    CString& operator=(const CString& rhs);

    CString& operator=(CString&& rhs) noexcept {
        this->swap(rhs);
        return *this;
    }

    ~CString() noexcept;

    void swap(CString& other) noexcept {
        // don't use std::swap(), we don't want an STL dependency in this file
        auto *temp = mCStr;
        mCStr = other.mCStr;
        other.mCStr = temp;
    }

    pointer c_str() noexcept { return mCStr; }
    const_pointer c_str() const noexcept { return const_cast<CString*>(this)->c_str(); }
    const_pointer c_str_safe() const noexcept { return mData ? c_str() : ""; }
    const_pointer data() const noexcept { return c_str(); }
    pointer data() noexcept { return c_str(); }
    size_type size() const noexcept { return mData ? mData[-1].length : 0; }
    size_type length() const noexcept { return size(); }
    bool empty() const noexcept { return size() == 0; }

    iterator begin() noexcept { return c_str(); }
    iterator end() noexcept { return begin() + length(); }
    const_iterator begin() const noexcept { return data(); }
    const_iterator end() const noexcept { return begin() + length(); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    // replace
    template<size_t N>
    CString& replace(size_type const pos,
            size_type const len, const StringLiteral<N>& str) & noexcept {
        return replace(pos, len, str, N - 1);
    }

    CString& replace(size_type const pos, size_type const len, const CString& str) & noexcept {
        return replace(pos, len, str.c_str_safe(), str.size());
    }

    template<size_t N>
    CString&& replace(size_type const pos,
            size_type const len, const StringLiteral<N>& str) && noexcept {
        return std::move(replace(pos, len, str));
    }

    CString&& replace(size_type const pos, size_type const len, const CString& str) && noexcept {
        return std::move(replace(pos, len, str));
    }

    // insert
    template<size_t N>
    CString& insert(size_type const pos, const StringLiteral<N>& str) & noexcept {
        return replace(pos, 0, str);
    }

    CString& insert(size_type const pos, const CString& str) & noexcept {
        return replace(pos, 0, str);
    }

    template<size_t N>
    CString&& insert(size_type const pos, const StringLiteral<N>& str) && noexcept {
        return std::move(*this).replace(pos, 0, str);
    }

    CString&& insert(size_type const pos, const CString& str) && noexcept {
        return std::move(*this).replace(pos, 0, str);
    }

    // append
    template<size_t N>
    CString& append(const StringLiteral<N>& str) & noexcept {
        return insert(length(), str);
    }

    CString& append(const CString& str) & noexcept {
        return insert(length(), str);
    }

    template<size_t N>
    CString&& append(const StringLiteral<N>& str) && noexcept {
        return std::move(*this).insert(length(), str);
    }

    CString&& append(const CString& str) && noexcept {
        return std::move(*this).insert(length(), str);
    }


    const_reference operator[](size_type const pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    reference operator[](size_type const pos) noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    const_reference at(size_type const pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    reference at(size_type const pos) noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    reference front() noexcept {
        assert(size());
        return begin()[0];
    }

    const_reference front() const noexcept {
        assert(size());
        return begin()[0];
    }

    reference back() noexcept {
        assert(size());
        return begin()[size() - 1];
    }

    const_reference back() const noexcept {
        assert(size());
        return begin()[size() - 1];
    }

    // placement new declared as "throw" to avoid the compiler's null-check
    void* operator new(size_t, void* ptr) {
        assert(ptr);
        return ptr;
    }

    struct Hasher : private hashCStrings {
        typedef CString argument_type;
        typedef size_t result_type;
        result_type operator()(const argument_type& s) const noexcept {
            return hashCStrings::operator()(s.c_str());
        }
    };

private:
    CString& replace(size_type pos, size_type len, char const* str, size_t l) & noexcept;

#if !defined(NDEBUG)
    friend io::ostream& operator<<(io::ostream& out, const CString& rhs);
#endif

    struct Data {
        size_type length;
    };

    // mCStr points to the C-string or nullptr. if non-null, mCStr is preceded by the string's size
    union {
        value_type *mCStr = nullptr;
        Data* mData; // Data is stored at mData[-1]
    };

    int compare(const CString& rhs) const noexcept {
        auto const l = std::string_view{data(), size()};
        auto const r = std::string_view{rhs.data(), rhs.size()};
        return l.compare(r);
    }

    friend bool operator==(CString const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(CString const& lhs, CString const& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend bool operator<(CString const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator>(CString const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>=(CString const& lhs, CString const& rhs) noexcept {
        return !(lhs < rhs);
    }
    friend bool operator<=(CString const& lhs, CString const& rhs) noexcept {
        return !(lhs > rhs);
    }
};

// Implement this for your type for automatic conversion to CString. Failing to do so leads
// to a compile-time failure.
template<typename T>
CString to_string(T value) noexcept;

// ------------------------------------------------------------------------------------------------

template <size_t N>
class UTILS_PUBLIC FixedSizeString {
public:
    using value_type      = char;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    static_assert(N > 0);

    FixedSizeString() noexcept = default;
    explicit FixedSizeString(const char* str) noexcept {
        strncpy(mData, str, N - 1); // leave room for the null terminator
    }

    const_pointer c_str() const noexcept { return mData; }
    pointer c_str() noexcept { return mData; }

private:
    value_type mData[N] = {};
};

} // namespace utils

#endif // TNT_UTILS_CSTRING_H
