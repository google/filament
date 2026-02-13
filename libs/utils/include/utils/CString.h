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
#include <type_traits>
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

template <size_t N>
using StringLiteral = const char[N];

// ------------------------------------------------------------------------------------------------

class UTILS_PUBLIC CString {
    static constexpr bool TRACK_AND_LOG_ALLOCATIONS = false;

    template<typename T>
    static constexpr bool is_char_pointer_v =
        std::is_pointer_v<T> && std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>>;

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

    CString() noexcept {
        track(true);
    }

    // Allocates memory and appends a null. This constructor can be used to hold arbitrary data
    // inside the string (i.e. it can contain nulls or non-ASCII encodings).
    CString(const char* cstr, size_t length);

    // Allocates memory for a string of size length plus space for the null terminating character.
    // Also initializes the memory to 0. This constructor can be used to hold arbitrary data
    // inside the string.
    explicit CString(size_t length);

    // Conversion from std::string_view
    explicit CString(const std::string_view& str)
        : CString(str.data(), str.size()) {
    }

    // Allocates memory and copies traditional C string content. Unlike the above constructor, this
    // does not allow embedded nulls. This is explicit because this operation is costly.
    // This is a template to ensure it's not preferred over the string literal constructor below.
    template<typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    explicit CString(T cstr) : CString(cstr, cstr ? strlen(cstr) : 0) {
        track(true);
    }

    // The string can't have NULs in it.
    template<size_t N>
    CString(StringLiteral<N> const& other) noexcept // NOLINT(google-explicit-constructor)
            : CString(other, N - 1) {
        track(true);
    }

    // This constructor can be used if the string has NULs in it.
    template<size_t N>
    CString(StringLiteral<N> const& other, size_t const length) noexcept
            : CString(other, length) {
        track(true);
    }

    CString(StaticString const& other) noexcept // NOLINT(*-explicit-constructor)
        : CString(other.c_str(), other.length()) {
        track(true);
    }

    CString(const CString& rhs);

    CString(CString&& rhs) noexcept {
        track(true);
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
    CString& replace(size_type const pos, size_type const len, const StringLiteral<N>& str) & noexcept {
        return replace(pos, len, str, N - 1);
    }

    CString& replace(size_type const pos, size_type const len, const CString& str) & noexcept {
        return replace(pos, len, str.c_str_safe(), str.size());
    }

    template <typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString& replace(size_type pos, size_type len, T str) & noexcept {
        if (str) {
            return replace(pos, len, str, strlen(str));
        }
        return replace(pos, len, "", 0);
    }

    template<size_t N>
    CString&& replace(size_type pos, size_type len, const StringLiteral<N>& str) && noexcept {
        this->replace(pos, len, str);
        return std::move(*this);
    }

    CString&& replace(size_type const pos, size_type const len, const CString& str) && noexcept {
        this->replace(pos, len, str);
        return std::move(*this);
    }

    template <typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString&& replace(size_type pos, size_type len, T str) && noexcept {
        this->replace(pos, len, str);
        return std::move(*this);
    }


    // insert
    CString& insert(size_type const pos, char const c) & noexcept {
        const char s[1] = { c };
        return replace(pos, 0, s, 1);
    }

    template<size_t N>
    CString& insert(size_type const pos, const StringLiteral<N>& str) & noexcept {
        return replace(pos, 0, str, N - 1);
    }

    CString& insert(size_type const pos, const CString& str) & noexcept {
        return replace(pos, 0, str.c_str_safe(), str.size());
    }

    template <typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString& insert(size_type pos, T str) & noexcept {
        if (str) {
            return replace(pos, 0, str, strlen(str));
        }
        return *this;
    }

    CString&& insert(size_type const pos, char const c) && noexcept {
        this->insert(pos, c);
        return std::move(*this);
    }

    template<size_t N>
    CString&& insert(size_type pos, const StringLiteral<N>& str) && noexcept {
        this->insert(pos, str);
        return std::move(*this);
    }

    CString&& insert(size_type const pos, const CString& str) && noexcept {
        this->insert(pos, str);
        return std::move(*this);
    }

    template <typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString&& insert(size_type pos, T str) && noexcept {
        this->insert(pos, str);
        return std::move(*this);
    }


    // append
    CString& append(char const c) & noexcept {
        return insert(length(), c);
    }

    template<size_t N>
    CString& append(const StringLiteral<N>& str) & noexcept {
        return insert(length(), str);
    }

    CString& append(const CString& str) & noexcept {
        return insert(length(), str);
    }

    template<typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString& append(T str) & noexcept {
        return insert(length(), str);
    }

    CString&& append(char const c) && noexcept {
        this->append(c);
        return std::move(*this);
    }

    template<size_t N>
    CString&& append(const StringLiteral<N>& str) && noexcept {
        this->append(str);
        return std::move(*this);
    }

    CString&& append(const CString& str) && noexcept {
        this->append(str);
        return std::move(*this);
    }

    template<typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString&& append(T str) && noexcept {
        this->append(str);
        return std::move(*this);
    }

    // operator+=
    CString& operator+=(char const c) & noexcept {
        return append(c);
    }

    CString& operator+=(const CString& str) & noexcept {
        return append(str);
    }
    template<size_t N>
    CString& operator+=(const StringLiteral<N>& str) & noexcept {
        return append(str);
    }
    template <typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    CString& operator+=(T str) & noexcept {
        return append(str);
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

    // conversion to std::string_view
    operator std::string_view() const noexcept {
        return std::string_view{data(), size()};
    }

private:
    static void do_tracking(bool ctor);
    static void track(bool ctor) {
        if constexpr (TRACK_AND_LOG_ALLOCATIONS) {
            do_tracking(ctor);
        }
    }

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

    int compare(const std::string_view& rhs) const noexcept {
        return std::string_view{data(), size()}.compare(rhs);
    }

    int compare(const CString& rhs) const noexcept {
        return compare(std::string_view{rhs.data(), rhs.size()});
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

    friend bool operator==(CString const& lhs, std::string_view const& rhs) noexcept {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator==(std::string_view const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(CString const& lhs, std::string_view const& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend bool operator!=(std::string_view const& lhs, CString const& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend bool operator<(CString const& lhs, std::string_view const& rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator<(std::string_view const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator>(CString const& lhs, std::string_view const& rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>(std::string_view const& lhs, CString const& rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>=(CString const& lhs, std::string_view const& rhs) noexcept {
        return !(lhs < rhs);
    }
    friend bool operator>=(std::string_view const& lhs, CString const& rhs) noexcept {
        return !(lhs < rhs);
    }
    friend bool operator<=(CString const& lhs, std::string_view const& rhs) noexcept {
        return !(lhs > rhs);
    }
    friend bool operator<=(std::string_view const& lhs, CString const& rhs) noexcept {
        return !(lhs > rhs);
    }
};

// operator+
inline CString operator+(CString lhs, const CString& rhs) {
    lhs += rhs;
    return lhs;
}

inline CString operator+(CString lhs, const char* rhs) {
    lhs += rhs;
    return lhs;
}

inline CString operator+(const char* lhs, CString rhs) {
    rhs.insert(0, lhs);
    return rhs;
}

inline CString operator+(CString lhs, char const rhs) {
    lhs += rhs;
    return lhs;
}

inline CString operator+(char const lhs, CString rhs) {
    rhs.insert(0, lhs);
    return rhs;
}

// CString vs StringLiteral
template<size_t N>
 bool operator==(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} == std::string_view{rhs, N - 1};
}
template<size_t N>
 bool operator!=(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} != std::string_view{rhs, N - 1};
}
template<size_t N>
 bool operator<(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} < std::string_view{rhs, N - 1};
}
template<size_t N>
 bool operator>(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} > std::string_view{rhs, N - 1};
}
template<size_t N>
 bool operator<=(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} <= std::string_view{rhs, N - 1};
}
template<size_t N>
 bool operator>=(CString const& lhs, const StringLiteral<N>& rhs) noexcept {
    return std::string_view{lhs.data(), lhs.size()} >= std::string_view{rhs, N - 1};
}

// StringLiteral vs CString
template<size_t M>
 bool operator==(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} == std::string_view{rhs.data(), rhs.size()};
}
template<size_t M>
 bool operator!=(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} != std::string_view{rhs.data(), rhs.size()};
}
template<size_t M>
 bool operator<(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} < std::string_view{rhs.data(), rhs.size()};
}
template<size_t M>
 bool operator>(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} > std::string_view{rhs.data(), rhs.size()};
}
template<size_t M>
 bool operator<=(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} <= std::string_view{rhs.data(), rhs.size()};
}
template<size_t M>
 bool operator>=(const StringLiteral<M>& lhs, CString const& rhs) noexcept {
    return std::string_view{lhs, M - 1} >= std::string_view{rhs.data(), rhs.size()};
}


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

// heterogeneous lookup support for associative containers
namespace std {
    template <>
    struct hash<utils::CString> {
        using is_transparent = void; // Enable heterogeneous lookup

        size_t operator()(const utils::CString& k) const noexcept {
            return compute_hash(std::string_view(k));
        }

        template <typename T, typename = std::enable_if_t<
            std::is_convertible_v<T, std::string_view> &&
            !std::is_same_v<std::decay_t<T>, utils::CString>>>
        size_t operator()(const T& k) const noexcept {
            return compute_hash(std::string_view(k));
        }

    private:
        size_t compute_hash(std::string_view k) const noexcept {
            size_t hash = 5381;
            for (char const c : k) {
                hash = (hash * 33u) ^ size_t(c);
            }
            return hash;
        }
    };

    template<>
    struct equal_to<utils::CString> {
        using is_transparent = void; // Enable heterogeneous lookup

        template <typename T, typename U>
        bool operator()(const T& lhs, const U& rhs) const {
            return lhs == rhs;
        }
    };
}

#endif // TNT_UTILS_CSTRING_H
