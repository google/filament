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

#ifndef SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_
#define SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_

#include "dawn/common/ityp_bitset.h"
#include "dawn/native/EnumClassBitmasks.h"

namespace dawn::native {

template <typename T>
class EnumMaskIterator final {
    static constexpr size_t N = EnumBitmaskSize<T>::value;
    static_assert(N > 0);

    using U = std::underlying_type_t<T>;

  public:
    explicit EnumMaskIterator(const T& mask) : mBitSet(static_cast<U>(mask)) {
        // If you hit this DAWN_ASSERT it means that you forgot to update EnumBitmaskSize<T>::value;
        DAWN_ASSERT(U(mask) == 0 || Log2(uint64_t(U(mask))) < N);
    }

    class Iterator final {
      public:
        explicit Iterator(const typename ityp::bitset<U, N>::Iterator& iter) : mIter(iter) {}

        Iterator& operator++() {
            ++mIter;
            return *this;
        }

        bool operator==(const Iterator& other) const = default;

        T operator*() const {
            U value = *mIter;
            return static_cast<T>(U(1) << value);
        }

      private:
        typename ityp::bitset<U, N>::Iterator mIter;
    };

    Iterator begin() const { return Iterator(mBitSet.begin()); }

    Iterator end() const { return Iterator(mBitSet.end()); }

  private:
    ityp::bitset<U, N> mBitSet;
};

template <typename T>
EnumMaskIterator<T> IterateEnumMask(const T& mask) {
    return EnumMaskIterator<T>(mask);
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_
