// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_UTILS_NUMERIC_H_
#define SRC_UTILS_NUMERIC_H_

#include <concepts>
#include <limits>
#include <type_traits>
#include <utility>

#include "src/utils/assert.h"
#include "src/utils/underlying_type.h"

namespace dawn {

// Checked conversion between differently sized integer-likes (i.e. all the types that can be used
// by ityp: integers, TypedIntegers, and enum classes). This is only defined for unsigned types
// because that is all that is needed at the time of writing, however eventually we will want to use
// this more widely, and we'll need to upgrade it (and the tests) to allow signed types.
template <HasUnderlyingType Dst, HasUnderlyingType Src>
constexpr inline Dst checked_cast(const Src& value) {
    using ISrc = UnderlyingType<Src>;
    using IDst = UnderlyingType<Dst>;
    // The compiler seems to be able to optimize away this CHECK, for Src/Dst pairs that can never
    // fail (verified in Compiler Explorer with plain integers and enum classes).
    //
    // Note, we choose not to disallow checked_cast for casts that are always safe, even though that
    // would guide authors toward static_casts that are statically-guaranteed safe. This is because
    // when used with things like size_t, which one you would need to use could differ by bitness.
    ISrc valueISrc = static_cast<ISrc>(value);
    DAWN_CHECK(std::in_range<IDst>(valueISrc));
    return Dst{static_cast<IDst>(valueISrc)};
}

template <HasUnderlyingType Dst, HasUnderlyingType Src>
constexpr inline Dst dchecked_cast(const Src& value) {
    using ISrc = UnderlyingType<Src>;
    using IDst = UnderlyingType<Dst>;
    ISrc valueISrc = static_cast<ISrc>(value);
    DAWN_ASSERT(std::in_range<IDst>(valueISrc));
    return Dst{static_cast<IDst>(valueISrc)};
}

// Checked cast from an integer type to it's exactly opposite-signed type.
template <std::unsigned_integral T>
auto sign_cast(T x) {
    return checked_cast<std::make_signed_t<T>>(x);
}

template <std::signed_integral T>
auto sign_cast(T x) {
    return checked_cast<std::make_unsigned_t<T>>(x);
}

// Cast from an integer type to it's exactly opposite-signed type, only checked in debug builds.
template <std::unsigned_integral T>
auto sign_dcast(T x) {
    return dchecked_cast<std::make_signed_t<T>>(x);
}

template <std::signed_integral T>
auto sign_dcast(T x) {
    return dchecked_cast<std::make_unsigned_t<T>>(x);
}

template <std::integral From, std::integral To>
constexpr inline bool kIsCastAlwaysInRange =
    std::in_range<To>(std::numeric_limits<From>::lowest()) &&
    std::in_range<To>(std::numeric_limits<From>::max());

template <typename T>
bool inline IsDoubleValueRepresentable(double value) {
    if constexpr (std::is_same_v<T, float> || std::is_integral_v<T>) {
        // Following WebIDL 3.3.6.[EnforceRange] for integral
        // Following WebIDL 3.2.5.float for float
        // TODO(crbug.com/1396194): now follows what blink does but may need revisit.
        constexpr double kLowest = static_cast<double>(std::numeric_limits<T>::lowest());
        constexpr double kMax = static_cast<double>(std::numeric_limits<T>::max());
        return kLowest <= value && value <= kMax;
    } else {
        static_assert(std::is_same_v<T, float> || std::is_integral_v<T>, "Unsupported type");
    }
}

inline bool IsDoubleValueRepresentableAsF16(double value) {
    constexpr double kLowestF16 = -65504.0;
    constexpr double kMaxF16 = 65504.0;
    return kLowestF16 <= value && value <= kMaxF16;
}

}  // namespace dawn

#endif  // SRC_UTILS_NUMERIC_H_
