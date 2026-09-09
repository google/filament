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

#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn::ityp {

// ityp::array is a helper class that wraps std::array with the restriction that
// indices must be a particular type |Index|. Dawn uses multiple flat maps of
// index-->data, and this class helps ensure an indices cannot be passed interchangably
// to a flat map of a different type.
template <typename Index, typename Value, size_t Size>
class array : private ::std::array<Value, Size> {
    using I = UnderlyingType<Index>;
    using Base = ::std::array<Value, Size>;

    static_assert(HasUnsignedUnderlyingType<Index>, "Index type must be unsigned");
    static_assert(Size <= std::numeric_limits<I>::max());
    static_assert(Size <= std::numeric_limits<size_t>::max());

  public:
    constexpr array() = default;

    template <typename... Values>
    explicit(false) constexpr array(Values&&... values) : Base{std::forward<Values>(values)...} {}

    using Base::begin, Base::end;
    using Base::front, Base::back;

    using Base::data;
    using Base::empty;
    using Base::fill;

    constexpr Value& operator[](Index i) { return Base::operator[](checked_cast<size_t>(i)); }
    constexpr const Value& operator[](Index i) const {
        return Base::operator[](checked_cast<size_t>(i));
    }

    constexpr Value& at(Index i) { return Base::at(checked_cast<size_t>(i)); }
    constexpr const Value& at(Index i) const { return Base::at(checked_cast<size_t>(i)); }

    constexpr Index size() const { return Index(static_cast<I>(Base::size())); }

    constexpr bool operator==(const array<Index, Value, Size>& other) const {
        return static_cast<const Base&>(*this) == static_cast<const Base&>(other);
    }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_ARRAY_H_
