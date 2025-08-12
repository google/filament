// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_TYPEDINTEGER_H_
#define SRC_DAWN_COMMON_TYPEDINTEGER_H_

#include <compare>
#include <concepts>
#include <limits>
#include <ostream>
#include <type_traits>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn {

// TypedInteger is helper class that provides additional type safety in Debug.
//  - Integers of different (Tag, BaseIntegerType) may not be used interoperably
//  - Allows casts only to the underlying type.
//  - Integers of the same (Tag, BaseIntegerType) may be compared or assigned.
// This class helps ensure that the many types of indices in Dawn aren't mixed up and used
// interchangably.
// In Release builds, when DAWN_ENABLE_ASSERTS is not defined, TypedInteger is a passthrough
// typedef of the underlying type.
//
// Example:
//     using UintA = dawn::TypedInteger<struct TypeA, uint32_t>;
//     using UintB = dawn::TypedInteger<struct TypeB, uint32_t>;
//
//  in Release:
//     using UintA = uint32_t;
//     using UintB = uint32_t;
//
//  in Debug:
//     using UintA = detail::TypedIntegerImpl<struct TypeA, uint32_t>;
//     using UintB = detail::TypedIntegerImpl<struct TypeB, uint32_t>;
//
//     Assignment, construction, comparison, and arithmetic with TypedIntegerImpl are allowed
//     only for typed integers of exactly the same type. Further, they must be
//     created / cast explicitly; there is no implicit conversion.
//
//     UintA a(2);
//     uint32_t aValue = static_cast<uint32_t>(a);
//
namespace detail {
template <typename Tag, typename T>
class TypedIntegerImpl;
}  // namespace detail

template <typename Tag, typename T>
    requires std::integral<T>
#if defined(DAWN_ENABLE_ASSERTS)
using TypedInteger = detail::TypedIntegerImpl<Tag, T>;
#else
using TypedInteger = T;
#endif

namespace detail {
template <typename Tag, typename T>
class alignas(T) TypedIntegerImpl {
    static_assert(std::is_integral<T>::value, "TypedInteger must be integral");
    T mValue;

  public:
    constexpr TypedIntegerImpl() : mValue(0) {
        static_assert(alignof(TypedIntegerImpl) == alignof(T));
        static_assert(sizeof(TypedIntegerImpl) == sizeof(T));
    }

    // Construction from non-narrowing integral types.
    // TODO(425911085): Consider allowing construction from narrowing types, but assert
    // that it doesn't truncate.
    template <typename I>
        requires std::integral<I>
    explicit constexpr TypedIntegerImpl(I rhs) : mValue(static_cast<T>(rhs)) {
        static_assert(std::numeric_limits<I>::max() <= std::numeric_limits<T>::max());
        static_assert(std::numeric_limits<I>::min() >= std::numeric_limits<T>::min());
    }

    // Allow explicit casts to any integral type. If the type is smaller than T,
    // we assert that no truncation occurs.
    template <typename I>
        requires std::integral<I>
    explicit constexpr operator I() const {
        if constexpr (sizeof(I) < sizeof(T)) {
            // If downcasting, assert that the value is not truncated
            DAWN_ASSERT(static_cast<I>(this->mValue) == this->mValue);
        }
        return static_cast<I>(this->mValue);
    }

    // Same-tag TypedInteger comparison operators
    constexpr auto operator<=>(const TypedIntegerImpl& rhs) const = default;

    // Increment / decrement operators for for-loop iteration
    constexpr TypedIntegerImpl& operator++() {
        DAWN_ASSERT(this->mValue < std::numeric_limits<T>::max());
        ++this->mValue;
        return *this;
    }

    constexpr TypedIntegerImpl operator++(int) {
        TypedIntegerImpl ret = *this;

        DAWN_ASSERT(this->mValue < std::numeric_limits<T>::max());
        ++this->mValue;
        return ret;
    }

    constexpr TypedIntegerImpl& operator--() {
        DAWN_ASSERT(this->mValue > std::numeric_limits<T>::min());
        --this->mValue;
        return *this;
    }

    constexpr TypedIntegerImpl operator--(int) {
        TypedIntegerImpl ret = *this;

        DAWN_ASSERT(this->mValue > std::numeric_limits<T>::min());
        --this->mValue;
        return ret;
    }

    template <typename T2 = T>
        requires(std::unsigned_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) AddImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        // Overflow would wrap around
        DAWN_ASSERT(lhs.mValue + rhs.mValue >= lhs.mValue);
        return lhs.mValue + rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) AddImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        if (lhs.mValue > 0) {
            // rhs is positive: |rhs| is at most the distance between max and |lhs|.
            // rhs is negative: (positive + negative) won't overflow
            DAWN_ASSERT(rhs.mValue <= std::numeric_limits<T>::max() - lhs.mValue);
        } else {
            // rhs is postive: (negative + positive) won't underflow
            // rhs is negative: |rhs| isn't less than the (negative) distance between min
            // and |lhs|
            DAWN_ASSERT(rhs.mValue >= std::numeric_limits<T>::min() - lhs.mValue);
        }
        return lhs.mValue + rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::unsigned_integral<T> && std::same_as<T, T2>)
    static constexpr decltype(T(0) - T2(0)) SubImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        // Overflow would wrap around
        DAWN_ASSERT(lhs.mValue - rhs.mValue <= lhs.mValue);
        return lhs.mValue - rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T> && std::same_as<T, T2>)
    static constexpr decltype(T(0) - T2(0)) SubImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        if (lhs.mValue > 0) {
            // rhs is positive: positive minus positive won't overflow
            // rhs is negative: |rhs| isn't less than the (negative) distance between |lhs|
            // and max.
            DAWN_ASSERT(rhs.mValue >= lhs.mValue - std::numeric_limits<T>::max());
        } else {
            // rhs is positive: |rhs| is at most the distance between min and |lhs|
            // rhs is negative: negative minus negative won't overflow
            DAWN_ASSERT(rhs.mValue <= lhs.mValue - std::numeric_limits<T>::min());
        }
        return lhs.mValue - rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::unsigned_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) MulImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        DAWN_ASSERT(lhs.mValue == 0 || rhs.mValue == 0 ||
                    lhs.mValue <= (std::numeric_limits<T>::max() / rhs.mValue));
        return lhs.mValue * rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) MulImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
        if (lhs.mValue > 0) {
            if (rhs.mValue > 0) {
                DAWN_ASSERT(lhs.mValue <= (std::numeric_limits<T>::max() / rhs.mValue));
            } else {
                DAWN_ASSERT(rhs.mValue >= (std::numeric_limits<T>::min() / lhs.mValue));
            }
        } else {
            if (rhs.mValue > 0) {
                DAWN_ASSERT(lhs.mValue >= (std::numeric_limits<T>::min() / rhs.mValue));
            } else {
                DAWN_ASSERT((lhs.mValue == 0) ||
                            (rhs.mValue >= (std::numeric_limits<T>::max() / lhs.mValue)));
            }
        }
        return lhs.mValue * rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::unsigned_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) DivImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        DAWN_ASSERT(rhs.mValue != 0);
        return lhs.mValue / rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) DivImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT33-C.+Ensure+that+division+and+remainder+operations+do+not+result+in+divide-by-zero+errors
        DAWN_ASSERT(!(rhs.mValue == 0 ||
                      (rhs.mValue == -1 && lhs.mValue == std::numeric_limits<T>::min())));
        return lhs.mValue / rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::unsigned_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) ModImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        DAWN_ASSERT(rhs.mValue != 0);
        return lhs.mValue % rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T2> && std::same_as<T, T2>)
    static constexpr decltype(T(0) + T2(0)) ModImpl(TypedIntegerImpl<Tag, T> lhs,
                                                    TypedIntegerImpl<Tag, T2> rhs) {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT33-C.+Ensure+that+division+and+remainder+operations+do+not+result+in+divide-by-zero+errors
        DAWN_ASSERT(!(rhs.mValue == 0 ||
                      (rhs.mValue == -1 && lhs.mValue == std::numeric_limits<T>::min())));
        return lhs.mValue % rhs.mValue;
    }

    template <typename T2 = T>
        requires(std::signed_integral<T2> && std::same_as<T, T2>)
    constexpr TypedIntegerImpl operator-() const {
        // The negation of the most negative value cannot be represented.
        DAWN_ASSERT(this->mValue != std::numeric_limits<T>::min());
        return TypedIntegerImpl(-this->mValue);
    }

    constexpr TypedIntegerImpl operator+(TypedIntegerImpl rhs) const {
        auto result = AddImpl(*this, rhs);
        static_assert(std::is_same<T, decltype(result)>::value, "Use ityp::Add instead.");
        return TypedIntegerImpl(result);
    }

    constexpr TypedIntegerImpl operator-(TypedIntegerImpl rhs) const {
        auto result = SubImpl(*this, rhs);
        static_assert(std::is_same<T, decltype(result)>::value, "Use ityp::Sub instead.");
        return TypedIntegerImpl(result);
    }

    constexpr TypedIntegerImpl operator*(TypedIntegerImpl rhs) const {
        auto result = MulImpl(*this, rhs);
        return TypedIntegerImpl(result);
    }

    constexpr TypedIntegerImpl operator/(TypedIntegerImpl rhs) const {
        auto result = DivImpl(*this, rhs);
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl operator%(TypedIntegerImpl rhs) const {
        auto result = ModImpl(*this, rhs);
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl& operator+=(const TypedIntegerImpl& rhs) {
        *this = *this + rhs;
        return *this;
    }

    constexpr TypedIntegerImpl& operator-=(const TypedIntegerImpl& rhs) {
        *this = *this - rhs;
        return *this;
    }

    constexpr TypedIntegerImpl& operator*=(const TypedIntegerImpl& rhs) {
        *this = *this * rhs;
        return *this;
    }

    constexpr TypedIntegerImpl& operator/=(const TypedIntegerImpl& rhs) {
        *this = *this / rhs;
        return *this;
    }

    constexpr TypedIntegerImpl& operator%=(const TypedIntegerImpl& rhs) {
        *this = *this % rhs;
        return *this;
    }

    template <typename H>
    friend H AbslHashValue(H state, const TypedIntegerImpl& value) {
        H::combine(std::move(state), value.mValue);
        return std::move(state);
    }
};

template <typename Tag, typename T>
std::ostream& operator<<(std::ostream& os, TypedIntegerImpl<Tag, T> value) {
    os << static_cast<T>(value);
    return os;
}

}  // namespace detail
}  // namespace dawn

namespace std {

template <typename Tag, typename T>
class numeric_limits<dawn::detail::TypedIntegerImpl<Tag, T>> : public numeric_limits<T> {
  public:
    static dawn::detail::TypedIntegerImpl<Tag, T> max() noexcept {
        return dawn::detail::TypedIntegerImpl<Tag, T>(std::numeric_limits<T>::max());
    }
    static dawn::detail::TypedIntegerImpl<Tag, T> min() noexcept {
        return dawn::detail::TypedIntegerImpl<Tag, T>(std::numeric_limits<T>::min());
    }
};

}  // namespace std

namespace dawn::ityp {

// These helpers below are provided since the default arithmetic operators for small integer
// types like uint8_t and uint16_t return integers, not their same type. To avoid lots of
// casting or conditional code between Release/Debug. Callsites should use ityp::Add(a, b) and
// ityp::Sub(a, b) instead.

template <typename Tag, typename T>
constexpr ::dawn::detail::TypedIntegerImpl<Tag, T> Add(
    ::dawn::detail::TypedIntegerImpl<Tag, T> lhs,
    ::dawn::detail::TypedIntegerImpl<Tag, T> rhs) {
    return ::dawn::detail::TypedIntegerImpl<Tag, T>(
        static_cast<T>(::dawn::detail::TypedIntegerImpl<Tag, T>::AddImpl(lhs, rhs)));
}

template <typename Tag, typename T>
constexpr ::dawn::detail::TypedIntegerImpl<Tag, T> Sub(
    ::dawn::detail::TypedIntegerImpl<Tag, T> lhs,
    ::dawn::detail::TypedIntegerImpl<Tag, T> rhs) {
    return ::dawn::detail::TypedIntegerImpl<Tag, T>(
        static_cast<T>(::dawn::detail::TypedIntegerImpl<Tag, T>::SubImpl(lhs, rhs)));
}

template <typename Tag, typename T>
constexpr ::dawn::detail::TypedIntegerImpl<Tag, T> PlusOne(
    ::dawn::detail::TypedIntegerImpl<Tag, T> value) {
    T one = 1;
    return Add(value, ::dawn::detail::TypedIntegerImpl<Tag, T>(one));
}

template <typename T>
    requires std::integral<T>
constexpr T Add(T lhs, T rhs) {
    return static_cast<T>(lhs + rhs);
}

template <typename T>
    requires std::integral<T>
constexpr T Sub(T lhs, T rhs) {
    return static_cast<T>(lhs - rhs);
}

template <typename T>
    requires std::integral<T>
constexpr T PlusOne(T value) {
    return static_cast<T>(value + 1);
}

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_TYPEDINTEGER_H_
