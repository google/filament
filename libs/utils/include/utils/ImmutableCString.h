/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_UTILS_IMMUTABLECSTRING_H
#define TNT_UTILS_IMMUTABLECSTRING_H

#include <utils/compiler.h>

#include <utils/StaticString.h>
#include <utils/ostream.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace utils {


class UTILS_PUBLIC ImmutableCString {
    static constexpr bool TRACK_AND_LOG_ALLOCATIONS = false;

    template<typename T>
    static constexpr bool is_char_pointer_v =
        std::is_pointer_v<T> && std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>>;

public:
    using value_type      = char;
    using size_type       = uint32_t;
    using difference_type = int32_t;
    using const_reference = const value_type&;
    using const_pointer   = const value_type*;
    using const_iterator  = const value_type*;

    ImmutableCString() noexcept {
        track(true, mIsStatic);
    }

    // The string can't have NULs in it.
    template<size_t N>
    ImmutableCString(const char (&str)[N]) noexcept : mData(str), mSize(N - 1) { // NOLINT(*-explicit-constructor)
        track(true, mIsStatic);
    }

    // This constructor can be used if the string has NULs in it.
    template<size_t N>
    ImmutableCString(const char (&str)[N], size_t const length) noexcept
            : mData(str), mSize(length) {
        track(true, mIsStatic);
    }

    template<typename T, typename = std::enable_if_t<is_char_pointer_v<T>>>
    explicit ImmutableCString(T cstr) {
        if (cstr) {
            initializeFrom(cstr, strlen(cstr));
        }
        track(true, mIsStatic);
    }

    ImmutableCString(const char* cstr, size_t const length) {
        initializeFrom(cstr, length);
        track(true, mIsStatic);
    }

    ImmutableCString(StaticString const& str) // NOLINT(*-explicit-constructor)
        : mData(str.data()), mSize(str.size()) {
        track(true, mIsStatic);
    }

    ImmutableCString(const ImmutableCString& other) {
        if (other.mIsStatic) {
            mIsStatic = other.mIsStatic;
            mSize = other.mSize;
            mData = other.mData;
        } else {
            initializeFrom(other.mData, other.mSize);
        }
        track(true, mIsStatic);
    }

    ImmutableCString(ImmutableCString&& other) noexcept {
        track(true, mIsStatic);
        this->swap(other);
    }

    ImmutableCString& operator=(const ImmutableCString& other);

    ImmutableCString& operator=(ImmutableCString&& other) noexcept;

    ~ImmutableCString() {
        track(false, mIsStatic);
        if (!mIsStatic) {
            free(const_cast<char*>(mData));
        }
    }

    bool isStatic() const noexcept { return mIsStatic; }
    bool isDynamic() const noexcept { return !mIsStatic; }

    const_pointer c_str_safe() const noexcept { return mData; }
    const_pointer c_str() const noexcept { return mData; }
    const_pointer data() const noexcept { return mData; }
    size_type size() const noexcept { return mSize; }
    size_type length() const noexcept { return mSize; }
    bool empty() const noexcept { return mSize == 0; }

    const_iterator begin() const noexcept { return mData; }
    const_iterator end() const noexcept { return mData + mSize; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    const_reference operator[](size_type const pos) const noexcept {
        assert(pos < mSize);
        return mData[pos];
    }

    const_reference at(size_type const pos) const noexcept {
        assert(pos < mSize);
        return mData[pos];
    }

    const_reference front() const noexcept {
        assert(mSize > 0);
        return mData[0];
    }

    const_reference back() const noexcept {
        assert(mSize > 0);
        return mData[mSize - 1];
    }

    void swap(ImmutableCString& other) noexcept {
        std::swap(mData, other.mData);
        std::swap(mSize, other.mSize);
        std::swap(mIsStatic, other.mIsStatic);
    }

private:
    static void do_tracking(bool ctor, bool is_static);
    static void track(bool ctor, bool is_static) {
        if constexpr (TRACK_AND_LOG_ALLOCATIONS) {
            do_tracking(ctor, is_static);
        }
    }

#if !defined(NDEBUG)
    friend io::ostream& operator<<(io::ostream& out, const ImmutableCString& rhs);
#endif

    void initializeFrom(const char* cstr, size_t length);

    int compare(const ImmutableCString& rhs) const noexcept {
        return std::string_view{ mData, mSize }.compare({ rhs.mData, rhs.mSize });
    }

    char const* mData = "";
    uint32_t mSize = 0;
    bool mIsStatic = true;

    friend bool operator==(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) != 0;
    }
    friend bool operator<(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator>(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator<=(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) <= 0;
    }
    friend bool operator>=(const ImmutableCString& lhs, const ImmutableCString& rhs) noexcept {
        return lhs.compare(rhs) >= 0;
    }
};

static_assert(sizeof(ImmutableCString) <= 16, "ImmutableCString should be 16 bytes or less");

} // namespace utils

#endif //TNT_UTILS_IMMUTABLECSTRING_H
