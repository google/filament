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

#ifndef SRC_UTILS_TYPEDINTEGER_H_
#define SRC_UTILS_TYPEDINTEGER_H_

#include <compare>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>
#include <utility>

#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn {

// TypedInteger is helper class that provides additional type safety in Debug.
//  - Integers of different (Tag, BaseIntegerType) may not be used interoperably
//  - Allows casts only to the underlying type.
//  - Integers of the same (Tag, BaseIntegerType) may be compared or assigned.
// This class helps ensure that the many types of indices in Dawn aren't mixed up and used
// interchangably.
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

template <typename Tag, std::integral T>
using TypedInteger = detail::TypedIntegerImpl<Tag, T>;

namespace detail {
template <typename Tag, typename T>
class DAWN_TRIVIAL_ABI alignas(T) TypedIntegerImpl {
    static_assert(std::is_integral_v<T>, "TypedInteger must be integral");

  public:
    // Note that |mValue| here needs to be public in order to satisfy requirements in dawn::Span to
    // allow using TypedIntegers as NTTP template arguments for fixed extent Spans.
    T mValue;

    constexpr TypedIntegerImpl() : mValue(0) {
        static_assert(alignof(TypedIntegerImpl) == alignof(T));
        static_assert(sizeof(TypedIntegerImpl) == sizeof(T));
    }

    // Lossless conversion: primitive -> TypedInteger (constructor).
    // If you need a lossy (narrowing) conversion, use (d)checked_cast.
    template <std::integral Src>
        requires kIsCastAlwaysInRange<Src, T>
    explicit constexpr TypedIntegerImpl(Src src) : mValue(static_cast<T>(src)) {}

    // Lossless conversion: TypedInteger -> primitive (cast)
    // If you need a lossy (narrowing) conversion, use (d)checked_cast.
    template <std::integral Dst>
        requires kIsCastAlwaysInRange<T, Dst>
    explicit constexpr operator Dst() const {
        return static_cast<Dst>(this->mValue);
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

    constexpr TypedIntegerImpl operator-() const
        requires(std::signed_integral<T>)
    {
        // The negation of the most negative value cannot be represented.
        DAWN_ASSERT(this->mValue != std::numeric_limits<T>::min());
        return TypedIntegerImpl(-this->mValue);
    }

    constexpr TypedIntegerImpl operator+(TypedIntegerImpl rhs) const
        requires(std::unsigned_integral<T>)
    {
        // Overflow would wrap around
        DAWN_ASSERT(mValue + rhs.mValue >= mValue);
        T result = mValue + rhs.mValue;
        return TypedIntegerImpl{result};
    }
    constexpr TypedIntegerImpl operator+(TypedIntegerImpl rhs) const
        requires(std::signed_integral<T>)
    {
        if (mValue > 0) {
            // rhs is positive: |rhs| is at most the distance between max and |lhs|.
            // rhs is negative: (positive + negative) won't overflow
            DAWN_ASSERT(rhs.mValue <= std::numeric_limits<T>::max() - mValue);
        } else {
            // rhs is positive: (negative + positive) won't underflow
            // rhs is negative: |rhs| isn't less than the (negative) distance between min
            // and |lhs|
            DAWN_ASSERT(rhs.mValue >= std::numeric_limits<T>::min() - mValue);
        }
        T result = mValue + rhs.mValue;
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl operator-(TypedIntegerImpl rhs) const
        requires(std::unsigned_integral<T>)
    {
        // Overflow would wrap around
        DAWN_ASSERT(mValue - rhs.mValue <= mValue);
        T result = mValue - rhs.mValue;
        return TypedIntegerImpl{result};
    }
    constexpr TypedIntegerImpl operator-(TypedIntegerImpl rhs) const
        requires(std::signed_integral<T>)
    {
        if (mValue > 0) {
            // rhs is positive: positive minus positive won't overflow
            // rhs is negative: |rhs| isn't less than the (negative) distance between |lhs|
            // and max.
            DAWN_ASSERT(rhs.mValue >= mValue - std::numeric_limits<T>::max());
        } else {
            // rhs is positive: |rhs| is at most the distance between min and |lhs|
            // rhs is negative: negative minus negative won't overflow
            DAWN_ASSERT(rhs.mValue <= mValue - std::numeric_limits<T>::min());
        }
        T result = mValue - rhs.mValue;
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl operator*(TypedIntegerImpl rhs) const
        requires(std::unsigned_integral<T>)
    {
        DAWN_ASSERT(mValue == 0 || rhs.mValue == 0 ||
                    mValue <= (std::numeric_limits<T>::max() / rhs.mValue));
        T result = mValue * rhs.mValue;
        return TypedIntegerImpl{result};
    }
    constexpr TypedIntegerImpl operator*(TypedIntegerImpl rhs) const
        requires(std::signed_integral<T>)
    {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
        if (mValue > 0) {
            if (rhs.mValue > 0) {
                DAWN_ASSERT(mValue <= (std::numeric_limits<T>::max() / rhs.mValue));
            } else {
                DAWN_ASSERT(rhs.mValue >= (std::numeric_limits<T>::min() / mValue));
            }
        } else {
            if (rhs.mValue > 0) {
                DAWN_ASSERT(mValue >= (std::numeric_limits<T>::min() / rhs.mValue));
            } else {
                DAWN_ASSERT((mValue == 0) ||
                            (rhs.mValue >= (std::numeric_limits<T>::max() / mValue)));
            }
        }
        T result = mValue * rhs.mValue;
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl operator/(TypedIntegerImpl rhs) const
        requires(std::unsigned_integral<T>)
    {
        DAWN_ASSERT(rhs.mValue != 0);
        T result = mValue / rhs.mValue;
        return TypedIntegerImpl{result};
    }
    constexpr TypedIntegerImpl operator/(TypedIntegerImpl rhs) const
        requires(std::signed_integral<T>)
    {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT33-C.+Ensure+that+division+and+remainder+operations+do+not+result+in+divide-by-zero+errors
        DAWN_ASSERT(
            !(rhs.mValue == 0 || (rhs.mValue == -1 && mValue == std::numeric_limits<T>::min())));
        T result = mValue / rhs.mValue;
        return TypedIntegerImpl{result};
    }

    constexpr TypedIntegerImpl operator%(TypedIntegerImpl rhs) const
        requires(std::unsigned_integral<T>)
    {
        DAWN_ASSERT(rhs.mValue != 0);
        T result = mValue % rhs.mValue;
        return TypedIntegerImpl{result};
    }
    constexpr TypedIntegerImpl operator%(TypedIntegerImpl rhs) const
        requires(std::signed_integral<T>)
    {
        // https://wiki.sei.cmu.edu/confluence/display/c/INT33-C.+Ensure+that+division+and+remainder+operations+do+not+result+in+divide-by-zero+errors
        DAWN_ASSERT(
            !(rhs.mValue == 0 || (rhs.mValue == -1 && mValue == std::numeric_limits<T>::min())));
        T result = mValue % rhs.mValue;
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

    // Helper functions to avoid the need to do a double cast when we just want to add 1 to a typed
    // integer (when ++ or -- are not options).
    TypedIntegerImpl PlusOne() const { return *this + TypedIntegerImpl{T{1}}; }
    TypedIntegerImpl MinusOne() const { return *this - TypedIntegerImpl{T{1}}; }

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
    static constexpr dawn::detail::TypedIntegerImpl<Tag, T> max() noexcept {
        return dawn::detail::TypedIntegerImpl<Tag, T>(std::numeric_limits<T>::max());
    }
    static constexpr dawn::detail::TypedIntegerImpl<Tag, T> min() noexcept {
        return dawn::detail::TypedIntegerImpl<Tag, T>(std::numeric_limits<T>::min());
    }
};

}  // namespace std

#endif  // SRC_UTILS_TYPEDINTEGER_H_
