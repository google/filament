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

#ifndef SRC_DAWN_COMMON_RANGE_H_
#define SRC_DAWN_COMMON_RANGE_H_

#include <type_traits>
#include <utility>

namespace dawn {

// An iterator over a range of numbers. Simplified version of Python's range() iterator that works
// with any integer type (including ityp).
//
// Iterates over [0, end) in order:
//
//   for (auto i : Range(end)) { ... }
//
// Iterates over [start, end) in order:
//
//   for (auto i = Range(start, end)) { ... }
template <typename Integer>
class BeginEndRange {
  public:
    BeginEndRange(Integer begin, Integer end) : mBegin(begin), mEnd(end) {}

    class Iterator final {
      public:
        explicit Iterator(Integer value) : mValue(value) {}
        bool operator==(const Iterator& other) const { return other.mValue == mValue; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        Iterator& operator++() {
            mValue++;
            return *this;
        }
        Integer operator*() const { return mValue; }

      private:
        Integer mValue;
    };

    Iterator begin() const { return Iterator(mBegin); }
    Iterator end() const { return Iterator(mEnd); }

  private:
    Integer mBegin;
    Integer mEnd;
};

template <typename Integer>
BeginEndRange<Integer> Range(Integer end) {
    return {Integer{}, end};
}
template <typename Integer>
BeginEndRange<Integer> Range(Integer begin, Integer end) {
    return {begin, end};
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_RANGE_H_
