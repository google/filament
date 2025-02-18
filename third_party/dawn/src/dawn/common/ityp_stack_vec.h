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
#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

template <typename Index, typename Value, size_t StaticCapacity>
class stack_vec : private absl::InlinedVector<Value, StaticCapacity> {
    using I = UnderlyingType<Index>;
    using Base = absl::InlinedVector<Value, StaticCapacity>;
    static_assert(StaticCapacity <= std::numeric_limits<I>::max());

  public:
    stack_vec() : Base() {}
    explicit stack_vec(Index size) : Base() { Base::resize(static_cast<I>(size)); }

    Value& operator[](Index i) {
        DAWN_ASSERT(i < size());
        return Base::operator[](static_cast<I>(i));
    }

    constexpr const Value& operator[](Index i) const {
        DAWN_ASSERT(i < size());
        return Base::operator[](static_cast<I>(i));
    }

    void resize(Index size) { Base::resize(static_cast<I>(size)); }

    void reserve(Index size) { Base::reserve(static_cast<I>(size)); }

    Value* data() { return Base::data(); }

    const Value* data() const { return Base::data(); }

    Index size() const { return Index(static_cast<I>(Base::size())); }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_STACK_VEC_H_
