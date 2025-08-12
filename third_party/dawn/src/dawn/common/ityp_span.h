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

#ifndef SRC_DAWN_COMMON_ITYP_SPAN_H_
#define SRC_DAWN_COMMON_ITYP_SPAN_H_

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

// ityp::span is a helper class that wraps an unowned packed array of type |Value|.
// It stores the size and pointer to first element. It has the restriction that
// indices must be a particular type |Index|. This provides a type-safe way to index
// raw pointers.
template <typename Index, typename Value>
class span {
    using I = UnderlyingType<Index>;

  public:
    constexpr span() : mData(nullptr), mSize(0) {}
    constexpr span(Value* data, Index size) : mData(data), mSize(size) {}

    constexpr Value& operator[](Index i) const {
        DAWN_ASSERT(i < mSize);
        return mData[static_cast<I>(i)];
    }

    Value* data() noexcept { return mData; }

    const Value* data() const noexcept { return mData; }

    Value* begin() noexcept { return mData; }

    const Value* begin() const noexcept { return mData; }

    Value* end() noexcept { return mData + static_cast<I>(mSize); }

    const Value* end() const noexcept { return mData + static_cast<I>(mSize); }

    Value& front() {
        DAWN_ASSERT(mData != nullptr);
        DAWN_ASSERT(static_cast<I>(mSize) >= 0);
        return *mData;
    }

    const Value& front() const {
        DAWN_ASSERT(mData != nullptr);
        DAWN_ASSERT(static_cast<I>(mSize) >= 0);
        return *mData;
    }

    Value& back() {
        DAWN_ASSERT(mData != nullptr);
        DAWN_ASSERT(static_cast<I>(mSize) >= 0);
        return *(mData + static_cast<I>(mSize) - 1);
    }

    const Value& back() const {
        DAWN_ASSERT(mData != nullptr);
        DAWN_ASSERT(static_cast<I>(mSize) >= 0);
        return *(mData + static_cast<I>(mSize) - 1);
    }

    Index size() const { return mSize; }

  private:
    Value* mData;
    Index mSize;
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
