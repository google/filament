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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#ifndef SRC_DAWN_COMMON_ITYP_SPAN_H_
#define SRC_DAWN_COMMON_ITYP_SPAN_H_

#include <limits>
#include <span>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

// ityp::span is a helper class that wraps std::span<T, std::dynamic_extent>
// with the restriction that indices must be a particular type |Index|.
template <typename Index, typename Value>
class span : private ::std::span<Value> {
    using I = UnderlyingType<Index>;
    using Base = ::std::span<Value>;

  public:
    constexpr span() = default;
    constexpr span(Value* data, Index size) : Base{data, static_cast<I>(size)} {}

    constexpr Value& operator[](Index i) const { return Base::operator[](static_cast<I>(i)); }

    constexpr Index size() const {
        DAWN_ASSERT(std::numeric_limits<I>::max() >= Base::size());
        return Index(static_cast<I>(Base::size()));
    }

    using Base::data;

    using Base::begin;
    using Base::end;

    using Base::back;
    using Base::front;
};

// ityp::SpanFromUntyped<Index>(myValues, myValueCount) creates a span<Index, Value> from a C-style
// span that's without a TypedInteger index. It is useful at the interface between code that doesn't
// use ityp and code that does.
template <typename Index, typename Value>
span<Index, Value> SpanFromUntyped(Value* data, size_t size) {
    return {data, Index{static_cast<UnderlyingType<Index>>(size)}};
}

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_SPAN_H_
