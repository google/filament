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

#include <array>
#include <ranges>
#include <utility>

#include "src/dawn/common/ityp_vector.h"
#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

class ITypVectorTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;

    using Vector = ityp::vector<Key, Val>;

    // Check that ityp::vector can be used as a range.
    static_assert(std::ranges::contiguous_range<Vector>);
};

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using ITypVectorDeathTest = ITypVectorTest;

// Test creation and initialization of the vector.
TEST_F(ITypVectorTest, Creation) {
    // Default constructor initializes to 0
    {
        Vector vec;
        ASSERT_EQ(vec.size(), Key(0u));
    }

    // Size constructor initializes contents to 0
    {
        Vector vec(Key(10u));
        ASSERT_EQ(vec.size(), Key(10u));

        for (Key i(0u); i < Key(10u); ++i) {
            ASSERT_EQ(vec[i], Val(0u));
        }
    }

    // Size and initial value constructor initializes contents to the inital value
    {
        Vector vec(Key(10u), Val(7u));
        ASSERT_EQ(vec.size(), Key(10u));

        for (Key i(0u); i < Key(10u); ++i) {
            ASSERT_EQ(vec[i], Val(7u));
        }
    }

    // Initializer list constructor
    {
        Vector vec = {Val(2u), Val(8u), Val(1u)};
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));
    }
}

// Test copy construction/assignment
TEST_F(ITypVectorTest, CopyConstructAssign) {
    // Test the copy constructor
    {
        Vector rhs = {Val(2u), Val(8u), Val(1u)};

        Vector vec(rhs);
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));

        ASSERT_EQ(rhs.size(), Key(3u));
        ASSERT_EQ(rhs[Key(0u)], Val(2u));
        ASSERT_EQ(rhs[Key(1u)], Val(8u));
        ASSERT_EQ(rhs[Key(2u)], Val(1u));
    }

    // Test the copy assignment
    {
        Vector rhs = {Val(2u), Val(8u), Val(1u)};

        Vector vec = rhs;
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));

        ASSERT_EQ(rhs.size(), Key(3u));
        ASSERT_EQ(rhs[Key(0u)], Val(2u));
        ASSERT_EQ(rhs[Key(1u)], Val(8u));
        ASSERT_EQ(rhs[Key(2u)], Val(1u));
    }
}

// Test move construction/assignment
TEST_F(ITypVectorTest, MoveConstructAssign) {
    // Test the move constructor
    {
        Vector rhs = {Val(2u), Val(8u), Val(1u)};

        Vector vec(std::move(rhs));
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));
    }

    // Test the move assignment
    {
        Vector rhs = {Val(2u), Val(8u), Val(1u)};

        Vector vec = std::move(rhs);
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));
    }
}

// Test move construction/assignment
TEST_F(ITypVectorTest, Assign) {
    // Test assign with a count and value.
    {
        Vector vec;
        vec.assign(Key(3u), Val(2u));
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(2u));
        ASSERT_EQ(vec[Key(2u)], Val(2u));
    }
    // Test assign with an initializer list.
    {
        Vector vec;
        vec.assign({Val(2u), Val(8u), Val(1u)});
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));
    }
    // Test assign with two iterators.
    {
        std::array<Val, 3u> data = {Val(2u), Val(8u), Val(1u)};

        Vector vec;
        vec.assign(data.begin(), data.end());
        ASSERT_EQ(vec.size(), Key(3u));
        ASSERT_EQ(vec[Key(0u)], Val(2u));
        ASSERT_EQ(vec[Key(1u)], Val(8u));
        ASSERT_EQ(vec[Key(2u)], Val(1u));
    }
}

TEST_F(ITypVectorDeathTest, AssignTooManyElements) {
    using Key8 = TypedInteger<struct Key8T, uint8_t>;
    using Vector8 = ityp::vector<Key8, Val>;

    // Control case: assigning exactly at the limit of what Index can hold.
    {
        std::array<Val, 255> data = {};
        Vector8 vec;
        vec.assign(data.begin(), data.end());
    }
    // Error case: assigning exactly at the limit of what Index can hold.
    {
        std::array<Val, 256> data = {};
        Vector8 vec;
        EXPECT_DEATH_IF_SUPPORTED(vec.assign(data.begin(), data.end()), "");
    }

    // Note: not testing for assign with an std::initializer_list since it would require writing out
    // a list of 255 or 256 elements.
}

// Test that values can be set at an index and retrieved from the same index.
TEST_F(ITypVectorTest, Indexing) {
    Vector vec(Key(10u));
    {
        vec[Key(2u)] = Val(5u);
        vec[Key(1u)] = Val(9u);
        vec[Key(9u)] = Val(2u);

        ASSERT_EQ(vec[Key(2u)], Val(5u));
        ASSERT_EQ(vec[Key(1u)], Val(9u));
        ASSERT_EQ(vec[Key(9u)], Val(2u));
    }
    {
        vec.at(Key(4u)) = Val(5u);
        vec.at(Key(3u)) = Val(8u);
        vec.at(Key(1u)) = Val(7u);

        ASSERT_EQ(vec.at(Key(4u)), Val(5u));
        ASSERT_EQ(vec.at(Key(3u)), Val(8u));
        ASSERT_EQ(vec.at(Key(1u)), Val(7u));
    }
}

// Test that the vector can be iterated in order with a range-based for loop
TEST_F(ITypVectorTest, RangeBasedIteration) {
    Vector vec(Key(10u));

    // Assign in a non-const range-based for loop
    uint32_t i = 0;
    for (Val& val : vec) {
        val = Val(i);
    }

    // Check values in a const range-based for loop
    i = 0;
    for (Val val : static_cast<const Vector&>(vec)) {
        ASSERT_EQ(val, vec[Key(i++)]);
    }
}

// Test that begin/end/front/back/data return pointers/references to the correct elements.
TEST_F(ITypVectorTest, BeginEndFrontBackData) {
    Vector vec(Key(10u));

    // non-const versions
    ASSERT_EQ(&vec.front(), &vec[Key(0u)]);
    ASSERT_EQ(&vec.back(), &vec[Key(9u)]);
    ASSERT_EQ(vec.data(), &vec[Key(0u)]);

    // const versions
    const Vector& constVec = vec;
    ASSERT_EQ(&constVec.front(), &constVec[Key(0u)]);
    ASSERT_EQ(&constVec.back(), &constVec[Key(9u)]);
    ASSERT_EQ(constVec.data(), &constVec[Key(0u)]);
}

// Special case to make sure that operator[] works for ityp::vector<I, bool> as vector<bool> doesn't
// return a bool& for these (so that vector<bool> may use a bitfield internally).
TEST_F(ITypVectorTest, BoolVectorIndexing) {
    {
        ityp::vector<Key, bool> vec(Key(5u));
        const auto& const_vec = vec;

        vec[Key(2u)] = true;
        vec[Key(1u)] = true;
        vec[Key(4u)] = true;

        ASSERT_EQ(const_vec[Key(0u)], false);
        ASSERT_EQ(const_vec[Key(1u)], true);
        ASSERT_EQ(const_vec[Key(2u)], true);
        ASSERT_EQ(const_vec[Key(3u)], false);
        ASSERT_EQ(const_vec[Key(4u)], true);
    }

    {
        ityp::vector<Key, bool> vec(Key(5u));
        const auto& const_vec = vec;

        vec.at(Key(2u)) = true;
        vec.at(Key(1u)) = true;
        vec.at(Key(4u)) = true;

        ASSERT_EQ(const_vec.at(Key(0u)), false);
        ASSERT_EQ(const_vec.at(Key(1u)), true);
        ASSERT_EQ(const_vec.at(Key(2u)), true);
        ASSERT_EQ(const_vec.at(Key(3u)), false);
        ASSERT_EQ(const_vec.at(Key(4u)), true);
    }
}

// Out of bounds accesses should crash even in release (the underlying container
// should have asserts enabled).
TEST_F(ITypVectorDeathTest, OutOfBounds) {
    // MSVC doesn't have asserts (without _MSVC_STL_HARDENING).
    if constexpr (DAWN_COMPILER_IS(MSVC)) {
        GTEST_SKIP();
    }

    Vector vec(Key(10u), Val(7u));
    vec[Key(9u)];
    EXPECT_DEATH_IF_SUPPORTED(vec[Key(10u)], "");
    EXPECT_DEATH_IF_SUPPORTED(vec.at(Key(10u)), "");

    const Vector& constVec = vec;
    constVec[Key(9u)];
    EXPECT_DEATH_IF_SUPPORTED(constVec[Key(10u)], "");
    EXPECT_DEATH_IF_SUPPORTED(constVec.at(Key(10u)), "");
}

// If the index/size is 64-bit, it needs to be narrowed to size_t. Verify that's checked correctly.
TEST_F(ITypVectorDeathTest, OversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    using Key64 = TypedInteger<struct Key64T, uint64_t>;
    static constexpr Key64 kHugeKey64{0x1000'0000'0000'0000u};

    // Crash either due to OOM (on 64-bit) or due to narrowing (on 32-bit).
    EXPECT_DEATH_IF_SUPPORTED((ityp::vector<Key64, Val>(kHugeKey64)), "");
    EXPECT_DEATH_IF_SUPPORTED((ityp::vector<Key64, Val>(kHugeKey64, Val(7u))), "");

    ityp::vector<Key64, Val> vec(Key64(10u), Val(7u));

    vec[Key64(9u)];
    // Regular out-of-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[Key64(10u)], "");

    vec[Key64(0u)];
    // If this were cast to a 32-bit size_t without a check, it would be in-bounds.
    EXPECT_DEATH_IF_SUPPORTED(vec[kHugeKey64], "");

    EXPECT_DEATH_IF_SUPPORTED(vec.resize(kHugeKey64), "");
    EXPECT_DEATH_IF_SUPPORTED(vec.resize(kHugeKey64, Val(7u)), "");
    EXPECT_DEATH_IF_SUPPORTED(vec.reserve(kHugeKey64), "");
}

}  // anonymous namespace
}  // namespace dawn
