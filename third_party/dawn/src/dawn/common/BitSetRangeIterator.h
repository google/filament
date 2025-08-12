// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_
#define SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_

#include <bit>
#include <bitset>
#include <limits>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn {

// Similar to BitSetIterator but returns ranges of consecutive bits as (offset, size) pairs
template <size_t N, typename T>
class BitSetRangeIterator final {
  public:
    explicit BitSetRangeIterator(const std::bitset<N>& bitset);
    BitSetRangeIterator(const BitSetRangeIterator& other);
    BitSetRangeIterator& operator=(const BitSetRangeIterator& other);

    class Iterator final {
      public:
        constexpr explicit Iterator(const std::bitset<N>& bits,
                                    uint32_t offset = 0,
                                    uint32_t size = 0);
        Iterator& operator++();

        bool operator==(const Iterator& other) const = default;

        // Returns a pair of offset and size of the current range
        std::pair<T, size_t> operator*() const;

      private:
        void Advance();

        static constexpr size_t kBitsPerWord = sizeof(uint64_t) * 8;
        std::bitset<N> mBits;  // The original bitset shifted by mOffset + mSize.
        uint32_t mOffset;
        uint32_t mSize;
    };

    Iterator begin() const { return Iterator(mBits); }
    constexpr Iterator end() const { return Iterator(std::bitset<N>(), N, 0); }

  private:
    const std::bitset<N> mBits;
};

template <size_t N, typename T>
BitSetRangeIterator<N, T>::BitSetRangeIterator(const std::bitset<N>& bitset) : mBits(bitset) {}

template <size_t N, typename T>
BitSetRangeIterator<N, T>::BitSetRangeIterator(const BitSetRangeIterator& other)
    : mBits(other.mBits) {}

template <size_t N, typename T>
BitSetRangeIterator<N, T>& BitSetRangeIterator<N, T>::operator=(const BitSetRangeIterator& other) {
    mBits = other.mBits;
    return *this;
}

template <size_t N, typename T>
constexpr BitSetRangeIterator<N, T>::Iterator::Iterator(const std::bitset<N>& bits,
                                                        uint32_t offset,
                                                        uint32_t size)
    : mBits(bits), mOffset(offset), mSize(size) {
    // If the full range is set, directly compute the range. This avoids checking for the full range
    // in each call to Advance as that can only happen in the first iteration.
    if (mBits.all()) {
        mSize = N;
        mBits.reset();
        return;
    }

    Advance();
}

template <size_t N, typename T>
typename BitSetRangeIterator<N, T>::Iterator& BitSetRangeIterator<N, T>::Iterator::operator++() {
    Advance();
    return *this;
}

template <size_t N, typename T>
std::pair<T, size_t> BitSetRangeIterator<N, T>::Iterator::operator*() const {
    using U = UnderlyingType<T>;
    DAWN_ASSERT(static_cast<U>(mOffset) <= std::numeric_limits<U>::max());
    DAWN_ASSERT(static_cast<size_t>(mSize) <= std::numeric_limits<size_t>::max());
    return std::make_pair(static_cast<T>(static_cast<U>(mOffset)), static_cast<size_t>(mSize));
}

template <size_t N, typename T>
void BitSetRangeIterator<N, T>::Iterator::Advance() {
    constexpr std::bitset<N> kBlockMask(std::numeric_limits<uint64_t>::max());

    // Bits are currently shifted to mOffset + mSize.
    mOffset += mSize;
    mSize = 0;

    // There are no more 1s, so there are no more ranges.
    if (mBits.none()) {
        mOffset = N;
        return;
    }

    // Look for the next 1, shifting mBits as we go and accounting for it in mOffset.
    // The loop jumps in blocks of 64bit while the rest of the code dose the last sub-64bit count.
    {
        if constexpr (N > kBitsPerWord) {
            while ((mBits & kBlockMask).to_ullong() == 0) {
                mOffset += kBitsPerWord;
                mBits >>= kBitsPerWord;
            }
        }

        size_t nextBit = static_cast<size_t>(
            std::countr_zero(static_cast<uint64_t>((mBits & kBlockMask).to_ullong())));
        mOffset += nextBit;
        mBits >>= nextBit;
    }

    // Look for the next 0, shifting mBits as we go and accounting for it in mSize. There is a next
    // zero bit because the case with all bits set to 1 is handled in the iterator constructor.
    // The loop jumps in blocks of 64bit while the rest of the code dose the last sub-64bit count.
    DAWN_ASSERT(!mBits.all());
    {
        if constexpr (N > kBitsPerWord) {
            while ((mBits & kBlockMask).to_ullong() == kBlockMask) {
                mSize += kBitsPerWord;
                mBits >>= kBitsPerWord;
            }
        }

        size_t nextBit = static_cast<size_t>(
            std::countr_one(static_cast<uint64_t>((mBits & kBlockMask).to_ullong())));
        mSize += nextBit;
        mBits >>= nextBit;
    }
}

// Helper to avoid needing to specify the template parameter size
template <size_t N>
BitSetRangeIterator<N, uint32_t> IterateBitSetRanges(const std::bitset<N>& bitset) {
    return BitSetRangeIterator<N, uint32_t>(bitset);
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_
