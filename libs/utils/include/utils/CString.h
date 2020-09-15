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

#ifndef TNT_FILAMENT_CSTRING_H
#define TNT_FILAMENT_CSTRING_H

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

//! \privatesection
struct equalCStrings {
    typedef const char* first_argument_type;
    typedef const char* second_argument_type;
    typedef bool result_type;
    bool operator()(const char* lhs, const char* rhs) const noexcept {
        return !strcmp(lhs, rhs);
    }
};

//! \privatesection
struct lessCStrings {
    typedef const char* first_argument_type;
    typedef const char* second_argument_type;
    typedef bool result_type;
    result_type operator()(first_argument_type lhs, second_argument_type rhs) const noexcept {
        return strcmp(lhs, rhs) < 0;
    }
};

// This can be used to creates a string from a string literal -- w/o underlying allocations.
// e.g.:
//   StaticString s("Hello World!");
//
template <size_t N>
using StringLiteral = const char[N];

//! \publicsection
class UTILS_PUBLIC StaticString {
public:
    using value_type      = char;
    using size_type       = uint32_t;
    using difference_type = int32_t;
    using const_reference = const value_type&;
    using const_pointer   = const value_type*;
    using const_iterator  = const value_type*;

    constexpr StaticString() noexcept = default;

    // initialization from a string literal
    template <size_t N>
    StaticString(StringLiteral<N> const& other) noexcept // NOLINT(google-explicit-constructor)
        : mString(other),
          mLength(size_type(N - 1)),
          mHash(computeHash(other)) {
    }

    // assignment from a string literal
    template<size_t N>
    StaticString& operator=(StringLiteral<N> const& other) noexcept {
        mString = other;
        mLength = size_type(N - 1);
        mHash = computeHash(other);
        return *this;
    }

    // helper to make a StaticString from a C string that is known to be a string literal
    static constexpr StaticString make(const_pointer literal, size_t length) noexcept {
        StaticString r;
        r.mString = literal;
        r.mLength = size_type(length);
        size_type hash = 5381;
        while (int c = *literal++) {
            hash = (hash * 33u) ^ size_type(c);
        }
        r.mHash = hash;
        return r;
    }

    static StaticString make(const_pointer literal) noexcept {
        return make(literal, strlen(literal));
    }

    const_pointer c_str() const noexcept { return mString; }
    const_pointer data() const noexcept { return mString; }
    size_type size() const noexcept { return mLength; }
    size_type length() const noexcept { return mLength; }
    bool empty() const noexcept { return size() == 0; }
    void clear() noexcept { mString = nullptr; mLength = 0; }

    const_iterator begin() const noexcept { return mString; }
    const_iterator end() const noexcept { return mString + mLength; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    const_reference operator[](size_type pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    const_reference at(size_type pos) const noexcept {
        assert(pos < size());
        return begin()[pos];
    }

    const_reference front() const noexcept {
        assert(size());
        return begin()[0];
    }

    const_reference back() const noexcept {
        assert(size());
        return begin()[size() - 1];
    }

    size_type getHash() const noexcept { return mHash; }

private:
    const_pointer mString = nullptr;
    size_type mLength = 0;
    size_type mHash = 0;

    template<size_t N>
    static constexpr size_type computeHash(StringLiteral<N> const& s) noexcept {
        size_type hash = 5381;
        for (size_t i = 0; i < N - 1; i++) {
            hash = (hash * 33u) ^ size_type(s[i]);
        }
        return hash;
    }

    int compare(const StaticString& rhs) const noexcept;

    friend bool operator==(StaticString const& lhs, StaticString const& rhs) noexcept {
        return (lhs.data() == rhs.data()) ||
               ((lhs.size() == rhs.size()) && !strncmp(lhs.data(), rhs.data(), lhs.size()));
    }
    friend bool operator!=(StaticString const& lhs, StaticString const& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend bool operator<(StaticString const& lhs, StaticString const& rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator>(StaticString const& lhs, StaticString const& rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>=(StaticString const& lhs, StaticString const& rhs) noexcept {
        return !(lhs < rhs);
    }
    friend bool operator<=(StaticString const& lhs, StaticString const& rhs) noexcept {
        return !(lhs > rhs);
    }
};

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

    CString() noexcept = default;

    // cstr must be a null terminated string and length == strlen(cstr)
    CString(const char* cstr, size_t length);

    template<size_t N>
    explicit CString(StringLiteral<N> const& other) noexcept // NOLINT(google-explicit-constructor)
            : CString(other, N - 1) {
    }

    CString(StaticString const& s) : CString(s.c_str(), s.size()) {}

    CString(const CString& rhs);

    CString(CString&& rhs) noexcept {
        this->swap(rhs);
    }


    // this creates a CString from a null-terminated C string, this allocates memory and copies
    // its content. this is explicit because this operation is costly.
    explicit CString(const char* cstr);

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
    inline void* operator new(size_t size, void* ptr) {
        assert(ptr);
        return ptr;
    }

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

    friend bool operator==(CString const& lhs, StaticString const& rhs) noexcept {
        return (lhs.data() == rhs.data()) ||
               ((lhs.size() == rhs.size()) && !strncmp(lhs.data(), rhs.data(), lhs.size()));
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

} // namespace utils

// FIXME: how could we not include this one?
// needed for std::hash, since implementation is inline, this would not cause
// binaries incompatibilities if another STL version was used.
#include <functional>

namespace std {

//! \privatesection
template<>
struct hash<utils::CString> {
    typedef utils::CString argument_type;
    typedef size_t result_type;
    utils::hashCStrings hasher;
    size_t operator()(const utils::CString& s) const noexcept {
        return hasher(s.c_str());
    }
};

//! \privatesection
template<>
struct hash<utils::StaticString> {
    typedef utils::StaticString argument_type;
    typedef size_t result_type;
    size_t operator()(const utils::StaticString& s) const noexcept {
        return s.getHash();
    }
};

} // namespace std

#endif // TNT_FILAMENT_CSTRING_H
