// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_ENUMERATOR_H_
#define SRC_DAWN_COMMON_ENUMERATOR_H_

#include <type_traits>
#include <utility>

#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

// An iterator over a range that gives both the index and the value. It can be used like so:
//
//   ityp::span<Index, Value> span = ...;
//   for (auto [i, value] : Enumerate(span)) {
//       // Use i that's of type Index.
//       // Use value that's of type Value&.
//   }
template <typename Index, typename Value>
class EnumerateRange final {
  public:
    EnumerateRange(Index size, Value* begin) : mSize(size), mBegin(begin) {}

    class Iterator final {
      public:
        Iterator(Index index, Value* value) : mIndex(index), mValue(value) {}
        bool operator==(const Iterator& other) const { return other.mIndex == mIndex; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        Iterator& operator++() {
            mIndex++;
            mValue++;
            return *this;
        }
        std::pair<Index, Value&> operator*() const { return {mIndex, *mValue}; }

      private:
        Index mIndex;
        raw_ptr<Value, AllowPtrArithmetic> mValue;
    };

    Iterator begin() const { return Iterator(Index{}, mBegin.get()); }
    // Note that iterator comparison only uses mIndex, so we can save the computation of mValue for
    // the end() iterator.
    Iterator end() const { return Iterator(mSize, nullptr); }

  private:
    Index mSize;
    raw_ptr<Value> mBegin;
};

template <typename T,
          typename Index = decltype(std::declval<T>().size()),
          typename Value = std::remove_pointer_t<decltype(std::declval<T>().data())>>
EnumerateRange<Index, Value> Enumerate(T& v) {
    return {v.size(), v.data()};
}
}  // namespace dawn

#endif  // SRC_DAWN_COMMON_ENUMERATOR_H_
