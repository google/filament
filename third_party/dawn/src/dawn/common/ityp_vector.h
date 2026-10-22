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

#ifndef SRC_DAWN_COMMON_ITYP_VECTOR_H_
#define SRC_DAWN_COMMON_ITYP_VECTOR_H_

#include <cstddef>
#include <limits>
#include <vector>

#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn::ityp {

// ityp::vector is a helper class that wraps std::vector with the restriction that
// indices must be a particular type |Index|.
template <typename Index, typename Value>
class vector : private std::vector<Value> {
    using I = UnderlyingType<Index>;
    using Base = std::vector<Value>;

    static_assert(HasUnsignedUnderlyingType<Index>, "Index type must be unsigned");

  public:
    constexpr vector() : Base() {}
    constexpr explicit vector(Index size) : Base(checked_cast<size_t>(size)) {}
    constexpr vector(Index size, const Value& init) : Base(checked_cast<size_t>(size), init) {}
    constexpr vector(const vector& rhs) : Base(static_cast<const Base&>(rhs)) {}
    constexpr vector(vector&& rhs) : Base(static_cast<Base&&>(rhs)) {}
    constexpr vector(std::initializer_list<Value> init) : Base(init) {}

    constexpr vector& operator=(const vector& rhs) {
        Base::operator=(static_cast<const Base&>(rhs));
        return *this;
    }
    constexpr vector& operator=(vector&& rhs) noexcept {
        Base::operator=(static_cast<Base&&>(rhs));
        return *this;
    }

    using Base::begin, Base::end;
    using Base::cbegin, Base::cend;
    using Base::crbegin, Base::crend;
    using Base::front, Base::back;
    using Base::rbegin, Base::rend;

    using Base::clear;
    using Base::emplace;
    using Base::emplace_back;
    using Base::empty;
    using Base::erase;
    using Base::insert;
    using Base::push_back, Base::pop_back;

    // Manually forward data() - can't use `using` because it needs to be skipped for vector<bool>.
    constexpr auto data()
        requires(!std::is_same_v<Value, bool>)
    {
        return Base::data();
    }
    constexpr auto data() const
        requires(!std::is_same_v<Value, bool>)
    {
        return Base::data();
    }

    constexpr Base::reference operator[](Index i) {
        return Base::operator[](checked_cast<size_t>(i));
    }
    constexpr Base::const_reference operator[](Index i) const {
        return Base::operator[](checked_cast<size_t>(i));
    }

    constexpr Base::reference at(Index i) { return Base::at(checked_cast<size_t>(i)); }
    constexpr Base::const_reference at(Index i) const { return Base::at(checked_cast<size_t>(i)); }

    constexpr Index size() const { return Index(static_cast<I>(Base::size())); }

    constexpr void resize(Index size) { Base::resize(checked_cast<size_t>(size)); }

    constexpr void resize(Index size, const Value& value) {
        Base::resize(checked_cast<size_t>(size), value);
    }

    constexpr void reserve(Index size) { Base::reserve(checked_cast<size_t>(size)); }

    constexpr void assign(Index count, const Value& value) {
        Base::assign(checked_cast<size_t>(count), value);
    }

    template <class InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        Base::assign(first, last);
        if constexpr (sizeof(size_t) > sizeof(Index)) {
            DAWN_CHECK(Base::size() <=
                       size_t{UnderlyingType<Index>{std::numeric_limits<Index>::max()}});
        }
    }

    void assign(std::initializer_list<Value> ilist) {
        Base::assign(ilist);
        if constexpr (sizeof(size_t) > sizeof(Index)) {
            DAWN_CHECK(Base::size() <=
                       size_t{UnderlyingType<Index>{std::numeric_limits<Index>::max()}});
        }
    }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_VECTOR_H_
