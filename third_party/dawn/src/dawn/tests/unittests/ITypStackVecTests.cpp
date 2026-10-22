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

#include <ranges>
#include <utility>

#include "src/dawn/common/ityp_stack_vec.h"
#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

class ITypStackVecTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;

    using StackVec = ityp::stack_vec<Key, Val, 10>;

    // Check that ityp::stack_vec can be used as a range.
    static_assert(std::ranges::contiguous_range<StackVec>);
};

// Test creation and initialization of the stack_vec.
TEST_F(ITypStackVecTest, Creation) {
    // Default constructor initializes to 0
    {
        StackVec vec;
        ASSERT_EQ(vec.size(), Key(0u));
    }

    // Size constructor initializes contents to 0
    {
        StackVec vec(Key(10u));
        ASSERT_EQ(vec.size(), Key(10u));

        for (Key i(0u); i < Key(10u); ++i) {
            ASSERT_EQ(vec[i], Val(0u));
        }
    }
}

// Test that the vector can be iterated in order with a range-based for loop
TEST_F(ITypStackVecTest, RangeBasedIteration) {
    StackVec vec(Key(10u));

    // Assign in a non-const range-based for loop
    uint32_t i = 0;
    for (Val& val : vec) {
        val = Val(i);
    }

    // Check values in a const range-based for loop
    i = 0;
    for (Val val : static_cast<const StackVec&>(vec)) {
        ASSERT_EQ(val, vec[Key(i++)]);
    }
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using ITypStackVecDeathTest = ITypStackVecTest;

// Out of bounds accesses should crash even in release (the underlying container
// should have asserts enabled).
TEST_F(ITypStackVecDeathTest, OutOfBounds) {
    // MSVC doesn't have asserts (without _MSVC_STL_HARDENING).
    if constexpr (DAWN_COMPILER_IS(MSVC)) {
        GTEST_SKIP();
    }

    StackVec vec(Key(10u));
    EXPECT_DEATH_IF_SUPPORTED(vec[Key(10u)], "");

    const StackVec& constVec = vec;
    EXPECT_DEATH_IF_SUPPORTED(constVec[Key(10u)], "");
}

// If the index/size is 64-bit, it needs to be narrowed to size_t. Verify that's checked correctly.
TEST_F(ITypStackVecDeathTest, OversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeKey64{0x1000'0000'0000'0000u};

    // Crash either due to OOM (on 64-bit) or due to narrowing (on 32-bit).
    EXPECT_DEATH_IF_SUPPORTED((ityp::stack_vec<Key64, Val, 20>(kHugeKey64)), "");

    ityp::stack_vec<Key64, Val, 20> vec(Key64(10u));

    vec[Key64(9u)];
    // Regular out-of-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[Key64(10u)], "");

    vec[Key64(0u)];
    // If this were cast to a 32-bit size_t without a check, it would be in-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[kHugeKey64], "");

    EXPECT_DEATH_IF_SUPPORTED(vec.resize(kHugeKey64), "");
    EXPECT_DEATH_IF_SUPPORTED(vec.reserve(kHugeKey64), "");
}

}  // anonymous namespace
}  // namespace dawn
