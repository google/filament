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

#include <set>
#include <utility>
#include <vector>

#include "dawn/common/BitSetRangeIterator.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class BitSetRangeIteratorTest : public testing::Test {
  protected:
    struct Range {
        uint32_t offset;
        size_t size;
    };

    template <size_t N>
    void RunBitSetRangeTests(std::vector<Range> ranges) {
        std::bitset<N> stateBits;

        for (const auto& range : ranges) {
            for (uint32_t i = 0; i < range.size; ++i) {
                stateBits.set(range.offset + i);
            }
        }

        std::vector<Range> foundRanges;
        for (auto [offset, size] : IterateBitSetRanges(stateBits)) {
            foundRanges.push_back({offset, size});
        }

        EXPECT_EQ(ranges.size(), foundRanges.size());
        for (size_t i = 0; i < ranges.size(); ++i) {
            EXPECT_EQ(ranges[i].offset, foundRanges[i].offset);
            EXPECT_EQ(ranges[i].size, foundRanges[i].size);
        }
    }

    template <size_t N>
    void RunSingleBitTests() {
        RunBitSetRangeTests<N>({{N / 4u * 3u, 1}});
    }

    template <size_t N>
    void RunConsecutiveBitRangesTests() {
        std::bitset<N> stateBits;
        uint32_t bitSize = stateBits.size();
        // Ensure range no overlapping
        DAWN_ASSERT(bitSize >= 14);

        std::vector<std::pair<uint32_t, size_t>> expectedRanges;
        expectedRanges.push_back({2, 3});
        expectedRanges.push_back({bitSize / 2, 2});
        expectedRanges.push_back({bitSize / 2 + 3, 4});

        for (const auto& range : expectedRanges) {
            for (uint32_t i = 0; i < range.second; ++i) {
                stateBits.set(range.first + i);
            }
        }

        std::vector<std::pair<uint32_t, size_t>> foundRanges;
        for (auto range : IterateBitSetRanges(stateBits)) {
            foundRanges.push_back(range);
        }

        EXPECT_EQ(expectedRanges.size(), foundRanges.size());
        for (size_t i = 0; i < expectedRanges.size(); ++i) {
            EXPECT_EQ(expectedRanges[i].first, foundRanges[i].first);
            EXPECT_EQ(expectedRanges[i].second, foundRanges[i].second);
        }
    }

    template <size_t N>
    void RunNonLValueBitset() {
        std::bitset<N> stateBits;
        std::bitset<N> otherBits;

        uint32_t halfBitSize = stateBits.size() / 2;
        DAWN_ASSERT(halfBitSize >= 3);

        std::vector<std::pair<uint32_t, size_t>> expectedRanges;
        expectedRanges.push_back({halfBitSize - 2, 2});
        expectedRanges.push_back({halfBitSize + 1, 1});

        // Set up consecutive bits
        stateBits.set(halfBitSize - 2);
        stateBits.set(halfBitSize - 1);
        stateBits.set(halfBitSize);
        stateBits.set(halfBitSize + 1);

        // Set up second bitset with overlapping and non-overlapping bits
        otherBits.set(halfBitSize - 3);
        otherBits.set(halfBitSize - 2);
        otherBits.set(halfBitSize - 1);
        otherBits.set(halfBitSize + 1);
        otherBits.set(halfBitSize + 2);

        std::vector<std::pair<uint32_t, size_t>> foundRanges;
        for (auto range : IterateBitSetRanges(stateBits & otherBits)) {
            foundRanges.push_back(range);

            // Verify all bits in the range are set in both bitsets
            for (uint32_t i = 0; i < range.second; ++i) {
                EXPECT_TRUE(stateBits[range.first + i]);
                EXPECT_TRUE(otherBits[range.first + i]);
            }
        }

        EXPECT_EQ(expectedRanges.size(), foundRanges.size());
        for (size_t i = 0; i < expectedRanges.size(); ++i) {
            EXPECT_EQ(expectedRanges[i].first, foundRanges[i].first);
            EXPECT_EQ(expectedRanges[i].second, foundRanges[i].second);
        }
    }

    // Same value as BitSetRangeIterator.
    static constexpr size_t kBitsPerWord = sizeof(uint64_t) * 8;
};

// Test basic range iteration with single bits (each range has size 1)
TEST_F(BitSetRangeIteratorTest, SingleBit) {
    // Smaller than 1 word
    RunSingleBitTests<kBitsPerWord - 1>();

    // Equal to 1 word
    RunSingleBitTests<kBitsPerWord>();

    // Larger than 1 word
    RunSingleBitTests<kBitsPerWord * 2 - 1>();
}

// Test ranges with consecutive bits
TEST_F(BitSetRangeIteratorTest, ConsecutiveBitRanges) {
    // Smaller than 1 word
    RunConsecutiveBitRangesTests<kBitsPerWord - 1>();

    // Equal to 1 word
    RunConsecutiveBitRangesTests<kBitsPerWord>();

    // Larger than 1 word
    RunConsecutiveBitRangesTests<kBitsPerWord * 2 - 1>();
}

// Test an empty iterator
TEST_F(BitSetRangeIteratorTest, EmptySet) {
    std::bitset<kBitsPerWord> stateBits;
    auto iterator = IterateBitSetRanges(stateBits);
    EXPECT_EQ(iterator.end(), iterator.begin());
}

// Test iterating a result of combining two bitsets
TEST_F(BitSetRangeIteratorTest, NonLValueBitset) {
    // Smaller than 1 word
    RunNonLValueBitset<kBitsPerWord - 1>();

    // Equal to 1 word
    RunNonLValueBitset<kBitsPerWord>();

    // Larger than 1 word
    RunNonLValueBitset<kBitsPerWord * 2 - 1>();
}

// Test ranges that cross word boundaries
TEST_F(BitSetRangeIteratorTest, CrossWordBoundaryRanges) {
    // One range that crosses the boundary.
    // RunBitSetRangeTests<kBitsPerWord * 2>({{kBitsPerWord - 2, 4}});

    // One range that crosses the boundary then another one.
    RunBitSetRangeTests<kBitsPerWord * 3>(
        {{kBitsPerWord - 1, 2 + kBitsPerWord}, {kBitsPerWord * 2 + 2, 1}});
}

// Test ranges that start from first bit.
TEST_F(BitSetRangeIteratorTest, RangeWithZeroethBit) {
    // Smaller than 1 word
    RunBitSetRangeTests<kBitsPerWord - 1>({{0, kBitsPerWord - 1}});

    // Equal to 1 word
    RunBitSetRangeTests<kBitsPerWord>({{0, kBitsPerWord - 1}});

    // Larger than 1 word
    RunBitSetRangeTests<kBitsPerWord * 2>({{kBitsPerWord / 2, kBitsPerWord}});
}

}  // anonymous namespace
}  // namespace dawn
