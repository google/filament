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

#include <ranges>

#include "src/dawn/common/ityp_array.h"
#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

class ITypArrayTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;
    using Array = ityp::array<Key, Val, 10>;

    // Test that the expected array methods can be constexpr
    struct ConstexprTest {
        static constexpr Array kArr = {Val(0u), Val(1u), Val(2u), Val(3u), Val(4u),
                                       Val(5u), Val(6u), Val(7u), Val(8u), Val(9u)};

        static_assert(kArr[Key(3u)] == Val(3u));
        static_assert(kArr.at(Key(7u)) == Val(7u));
        static_assert(kArr.size() == Key(10u));
    };

    // Check that ityp::array can be used as a range.
    static_assert(std::ranges::contiguous_range<Array>);
};

// Test that values can be set at an index and retrieved from the same index.
TEST_F(ITypArrayTest, Indexing) {
    Array arr;
    {
        arr[Key(2u)] = Val(5u);
        arr[Key(1u)] = Val(9u);
        arr[Key(9u)] = Val(2u);

        ASSERT_EQ(arr[Key(2u)], Val(5u));
        ASSERT_EQ(arr[Key(1u)], Val(9u));
        ASSERT_EQ(arr[Key(9u)], Val(2u));
    }
    {
        arr.at(Key(4u)) = Val(5u);
        arr.at(Key(3u)) = Val(8u);
        arr.at(Key(1u)) = Val(7u);

        ASSERT_EQ(arr.at(Key(4u)), Val(5u));
        ASSERT_EQ(arr.at(Key(3u)), Val(8u));
        ASSERT_EQ(arr.at(Key(1u)), Val(7u));
    }
}

// Test that the array can be iterated in order with a range-based for loop
TEST_F(ITypArrayTest, RangeBasedIteration) {
    Array arr;

    // Assign in a non-const range-based for loop
    uint32_t i = 0;
    for (Val& val : arr) {
        val = Val(i);
    }

    // Check values in a const range-based for loop
    i = 0;
    for (Val val : static_cast<const Array&>(arr)) {
        ASSERT_EQ(val, arr[Key(i++)]);
    }
}

// Test that begin/end/front/back/data return pointers/references to the correct elements.
TEST_F(ITypArrayTest, BeginEndFrontBackData) {
    Array arr;

    // non-const versions
    ASSERT_EQ(&arr.front(), &arr[Key(0u)]);
    ASSERT_EQ(&arr.back(), &arr[Key(9u)]);
    ASSERT_EQ(arr.data(), &arr[Key(0u)]);

    // const versions
    const Array& constArr = arr;
    ASSERT_EQ(&constArr.front(), &constArr[Key(0u)]);
    ASSERT_EQ(&constArr.back(), &constArr[Key(9u)]);
    ASSERT_EQ(constArr.data(), &constArr[Key(0u)]);
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using ITypArrayDeathTest = ITypArrayTest;

// Out of bounds accesses should crash even in release (the underlying container
// should have asserts enabled).
TEST_F(ITypArrayDeathTest, OutOfBounds) {
    // MSVC doesn't have asserts (without _MSVC_STL_HARDENING).
    if constexpr (DAWN_COMPILER_IS(MSVC)) {
        GTEST_SKIP();
    }

    Array arr;
    EXPECT_DEATH_IF_SUPPORTED(arr[Key(10u)], "");
    EXPECT_DEATH_IF_SUPPORTED(arr.at(Key(10u)), "");

    const Array& constArr = arr;
    EXPECT_DEATH_IF_SUPPORTED(constArr[Key(10u)], "");
    EXPECT_DEATH_IF_SUPPORTED(constArr.at(Key(10u)), "");
}

// If the index/size is 64-bit, it needs to be narrowed to size_t. Verify that's checked correctly.
TEST_F(ITypArrayDeathTest, OversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeKey64{0x1'0000'0000LLU};

    ityp::array<Key64, Val, 10> vec;

    vec[Key64(9u)];
    // Regular out-of-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[Key64(10u)], "");

    vec[Key64(0u)];
    // If this were cast to a 32-bit size_t without a check, it would be in-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[kHugeKey64], "");
}

}  // anonymous namespace
}  // namespace dawn
