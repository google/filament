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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace utils {

//! \privatesection
struct hashCStrings {
    typedef const char* argument_type;
    typedef size_t result_type;
    result_type operator()(argument_type cstr) const noexcept {
        size_t hash = 5381;
        while (int c = *cstr++) {
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

    CString(const CString& rhs);

    CString(CString&& rhs) noexcept {
        this->swap(rhs);
    }


    CString& operator=(const CString& rhs);

    CString& operator=(CString&& rhs) noexcept {
        this->swap(rhs);
        return *this;
    }

    ~CString() noexcept {
        if (mData) {
            free(mData - 1);
        }
    }

    void swap(CString& other) noexcept {
        // don't use std::swap(), we don't want an STL dependency in this file
        auto *temp = mCStr;
        mCStr = other.mCStr;
        other.mCStr = temp;
    }

    const_pointer c_str() const noexcept { return mCStr; }
    pointer c_str() noexcept { return mCStr; }
    const_pointer c_str_safe() const noexcept { return mData ? c_str() : ""; }
    const_pointer data() const noexcept { return c_str(); }
    pointer data() noexcept { return c_str(); }
    size_type size() const noexcept { return mData ? mData[-1].length : 0; }
    size_type length() const noexcept { return size(); }
    bool empty() const noexcept { return size() == 0; }

    iterator begin() noexcept { return mCStr; }
    iterator end() noexcept { return begin() + length(); }
    const_iterator begin() const noexcept { return data(); }
    const_iterator end() const noexcept { return begin() + length(); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    CString& replace(size_type pos, size_type len, const CString& str) noexcept;
    CString& insert(size_type pos, const CString& str) noexcept { return replace(pos, 0, str); }

    const_reference operator[](size_type pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    reference operator[](size_type pos) noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    const_reference at(size_type pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    reference at(size_type pos) noexcept {
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
    inline void* operator new(size_t, void* ptr) {
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
    struct Data {
        size_type length;
    };

    // mCStr points to the C-string or nullptr. if non-null, mCStr is preceded by the string's size
    union {
        value_type *mCStr = nullptr;
        Data* mData; // Data is stored at mData[-1]
    };

    int compare(const CString& rhs) const noexcept {
        size_type lhs_size = size();
        size_type rhs_size = rhs.size();
        if (lhs_size < rhs_size) return -1;
        if (lhs_size > rhs_size) return 1;
        return strncmp(data(), rhs.data(), size());
    }

    friend bool operator==(CString const& lhs, CString const& rhs) noexcept {
        return (lhs.data() == rhs.data()) ||
               ((lhs.size() == rhs.size()) && !strncmp(lhs.data(), rhs.data(), lhs.size()));
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

// implement this for your type for automatic conversion to CString. Failing to do so leads
// to a compile-time failure.
template<typename T>
CString to_string(T value) noexcept;

} // namespace utils

#endif // TNT_UTILS_CSTRING_H
