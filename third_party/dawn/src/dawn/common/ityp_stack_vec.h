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

#ifndef SRC_DAWN_COMMON_ITYP_STACK_VEC_H_
#define SRC_DAWN_COMMON_ITYP_STACK_VEC_H_

#include <limits>

#include "absl/container/inlined_vector.h"
#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn::ityp {

template <typename Index, typename Value, size_t StaticCapacity>
class stack_vec : private absl::InlinedVector<Value, StaticCapacity> {
    using I = UnderlyingType<Index>;
    using Base = absl::InlinedVector<Value, StaticCapacity>;

    static_assert(HasUnsignedUnderlyingType<Index>, "Index type must be unsigned");
    static_assert(StaticCapacity <= std::numeric_limits<I>::max());

  public:
    constexpr stack_vec() : Base() {}
    constexpr explicit stack_vec(Index size) : Base() { Base::resize(checked_cast<size_t>(size)); }
    constexpr explicit stack_vec(Index size, Value value) : Base() {
        Base::resize(checked_cast<size_t>(size), value);
    }

    using Base::begin;
    using Base::data;
    using Base::empty;
    using Base::end;

    constexpr Value& operator[](Index i) {
#if !defined(ABSL_OPTION_HARDENED) || ABSL_OPTION_HARDENED == 0 || \
    defined(GOOGLE3_ABSL_HARDENED_BUILD) || defined(GOOGLE3_ABSL_HARDENED_BUILD_FAST)
        // Insert our own bounds check if not already enabled in absl
        DAWN_CHECK(i < size());
#endif
        return Base::operator[](checked_cast<size_t>(i));
    }
    constexpr const Value& operator[](Index i) const {
#if !defined(ABSL_OPTION_HARDENED) || ABSL_OPTION_HARDENED == 0 || \
    defined(GOOGLE3_ABSL_HARDENED_BUILD) || defined(GOOGLE3_ABSL_HARDENED_BUILD_FAST)
        // Insert our own bounds check if not already enabled in absl
        DAWN_CHECK(i < size());
#endif
        return Base::operator[](checked_cast<size_t>(i));
    }

    constexpr void resize(Index size) { Base::resize(checked_cast<size_t>(size)); }

    constexpr void reserve(Index size) { Base::reserve(checked_cast<size_t>(size)); }

    constexpr Index size() const { return Index(static_cast<I>(Base::size())); }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_STACK_VEC_H_
