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

#include <limits>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

// ityp::vector is a helper class that wraps std::vector with the restriction that
// indices must be a particular type |Index|.
template <typename Index, typename Value>
class vector : public std::vector<Value> {
    using I = UnderlyingType<Index>;
    using Base = std::vector<Value>;

  private:
    // Disallow access to base constructors and untyped index/size-related operators.
    using Base::Base;
    using Base::operator=;
    using Base::operator[];
    using Base::at;
    using Base::reserve;
    using Base::resize;
    using Base::size;

  public:
    vector() : Base() {}

    explicit vector(Index size) : Base(static_cast<I>(size)) {}

    vector(Index size, const Value& init) : Base(static_cast<I>(size), init) {}

    vector(const vector& rhs) : Base(static_cast<const Base&>(rhs)) {}

    vector(vector&& rhs) : Base(static_cast<Base&&>(rhs)) {}

    vector(std::initializer_list<Value> init) : Base(init) {}

    vector& operator=(const vector& rhs) {
        Base::operator=(static_cast<const Base&>(rhs));
        return *this;
    }

    vector& operator=(vector&& rhs) noexcept {
        Base::operator=(static_cast<Base&&>(rhs));
        return *this;
    }

    Value& operator[](Index i) {
        DAWN_ASSERT(i >= Index(0) && i < size());
        return Base::operator[](static_cast<I>(i));
    }

    constexpr const Value& operator[](Index i) const {
        DAWN_ASSERT(i >= Index(0) && i < size());
        return Base::operator[](static_cast<I>(i));
    }

    Value& at(Index i) {
        DAWN_ASSERT(i >= Index(0) && i < size());
        return Base::at(static_cast<I>(i));
    }

    constexpr const Value& at(Index i) const {
        DAWN_ASSERT(i >= Index(0) && i < size());
        return Base::at(static_cast<I>(i));
    }

    constexpr Index size() const {
        DAWN_ASSERT(std::numeric_limits<I>::max() >= Base::size());
        return Index(static_cast<I>(Base::size()));
    }

    void resize(Index size) { Base::resize(static_cast<I>(size)); }

    void reserve(Index size) { Base::reserve(static_cast<I>(size)); }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_VECTOR_H_
