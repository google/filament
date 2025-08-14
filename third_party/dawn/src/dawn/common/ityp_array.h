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

#ifndef SRC_DAWN_COMMON_ITYP_ARRAY_H_
#define SRC_DAWN_COMMON_ITYP_ARRAY_H_

#include <array>
#include <cstddef>
#include <limits>
#include <utility>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

// ityp::array is a helper class that wraps std::array with the restriction that
// indices must be a particular type |Index|. Dawn uses multiple flat maps of
// index-->data, and this class helps ensure an indices cannot be passed interchangably
// to a flat map of a different type.
template <typename Index, typename Value, size_t Size>
class array : private ::std::array<Value, Size> {
    using I = UnderlyingType<Index>;
    using Base = ::std::array<Value, Size>;

    static_assert(Size <= std::numeric_limits<I>::max());

  public:
    constexpr array() = default;

    template <typename... Values>
    // NOLINTNEXTLINE(runtime/explicit)
    constexpr array(Values&&... values) : Base{std::forward<Values>(values)...} {}

    constexpr Value& operator[](Index i) {
        I index = static_cast<I>(i);
        DAWN_ASSERT(index >= 0 && index < I(Size));
        return Base::operator[](index);
    }

    constexpr const Value& operator[](Index i) const {
        I index = static_cast<I>(i);
        DAWN_ASSERT(index >= 0 && index < I(Size));
        return Base::operator[](index);
    }

    Value& at(Index i) {
        I index = static_cast<I>(i);
        DAWN_ASSERT(index >= 0 && index < I(Size));
        return Base::at(index);
    }

    constexpr const Value& at(Index i) const {
        I index = static_cast<I>(i);
        DAWN_ASSERT(index >= 0 && index < I(Size));
        return Base::at(index);
    }

    typename Base::iterator begin() noexcept { return Base::begin(); }

    typename Base::const_iterator begin() const noexcept { return Base::begin(); }

    typename Base::iterator end() noexcept { return Base::end(); }

    typename Base::const_iterator end() const noexcept { return Base::end(); }

    constexpr Index size() const { return Index(I(Size)); }

    using Base::back;
    using Base::data;
    using Base::empty;
    using Base::fill;
    using Base::front;
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_ARRAY_H_
