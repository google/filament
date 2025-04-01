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

#ifndef TNT_UTILS_STATICSTRING_H
#define TNT_UTILS_STATICSTRING_H

#include <utils/compiler.h>

#include <type_traits>
#include <string_view>

#include <stddef.h>

namespace utils {

/**
 * @brief A lightweight string class that stores a pointer to a string literal and its size, without dynamic allocation.
 *
 * This class is designed to efficiently represent string literals. It does not allocate any memory
 * and instead relies on the compiler to manage the memory of the string literals.
 */
class UTILS_PUBLIC StaticString {
public:
    using value_type = std::string_view::value_type;
    using const_pointer = std::string_view::const_pointer;
    using const_reference = std::string_view::const_reference;
    using size_type = std::string_view::size_type;
    using const_iterator = std::string_view::const_iterator;

    // Constructor from string literal
    template <size_t M>
    constexpr StaticString(const char (&str)[M]) noexcept : mString(str, M - 1) {} // NOLINT(*-explicit-constructor)

    constexpr StaticString() noexcept = default;

    constexpr const_pointer c_str() const noexcept { return mString.data(); }
    constexpr const_pointer data() const noexcept { return mString.data(); }
    constexpr size_type size() const noexcept { return mString.size(); }
    constexpr size_type length() const noexcept { return mString.size(); }
    constexpr bool empty() const noexcept { return mString.empty(); }

    constexpr const_iterator begin() const noexcept { return mString.begin(); }
    constexpr const_iterator end() const noexcept { return mString.end(); }
    constexpr const_iterator cbegin() const noexcept { return mString.begin(); }
    constexpr const_iterator cend() const noexcept { return mString.end(); }

    constexpr const_reference operator[](size_type const pos) const noexcept {
        return mString[pos];
    }

    constexpr const_reference at(size_type pos) const {
        return mString[pos];
    }

    constexpr const_reference front() const noexcept {
        return mString.front();
    }

    constexpr const_reference back() const noexcept {
        return mString.back();
    }

    constexpr int compare(const StaticString& rhs) const noexcept {
        return mString.compare(rhs.mString);
    }

private:
    std::string_view mString;

    friend constexpr bool operator==(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString == rhs.mString;
    }

    friend constexpr bool operator!=(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString != rhs.mString;
    }

    friend constexpr bool operator<(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString < rhs.mString;
    }

    friend constexpr bool operator>(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString > rhs.mString;
    }

    friend constexpr bool operator<=(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString <= rhs.mString;
    }

    friend constexpr bool operator>=(const StaticString& lhs, const StaticString& rhs) noexcept {
        return lhs.mString >= rhs.mString;
    }
};

} // namespace utils

#endif // TNT_UTILS_STATICSTRING_H
