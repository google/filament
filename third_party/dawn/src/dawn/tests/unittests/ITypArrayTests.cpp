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

#include <gtest/gtest.h>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_array.h"

namespace dawn {
namespace {

class ITypArrayTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;
    using Array = ityp::array<Key, Val, 10>;

    // Test that the expected array methods can be constexpr
    struct ConstexprTest {
        static constexpr Array kArr = {Val(0), Val(1), Val(2), Val(3), Val(4),
                                       Val(5), Val(6), Val(7), Val(8), Val(9)};

        static_assert(kArr[Key(3)] == Val(3));
        static_assert(kArr.at(Key(7)) == Val(7));
        static_assert(kArr.size() == Key(10));
    };
};

// Test that values can be set at an index and retrieved from the same index.
TEST_F(ITypArrayTest, Indexing) {
    Array arr;
    {
        arr[Key(2)] = Val(5);
        arr[Key(1)] = Val(9);
        arr[Key(9)] = Val(2);

        ASSERT_EQ(arr[Key(2)], Val(5));
        ASSERT_EQ(arr[Key(1)], Val(9));
        ASSERT_EQ(arr[Key(9)], Val(2));
    }
    {
        arr.at(Key(4)) = Val(5);
        arr.at(Key(3)) = Val(8);
        arr.at(Key(1)) = Val(7);

        ASSERT_EQ(arr.at(Key(4)), Val(5));
        ASSERT_EQ(arr.at(Key(3)), Val(8));
        ASSERT_EQ(arr.at(Key(1)), Val(7));
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
    ASSERT_EQ(&arr.front(), &arr[Key(0)]);
    ASSERT_EQ(&arr.back(), &arr[Key(9)]);
    ASSERT_EQ(arr.data(), &arr[Key(0)]);

    // const versions
    const Array& constArr = arr;
    ASSERT_EQ(&constArr.front(), &constArr[Key(0)]);
    ASSERT_EQ(&constArr.back(), &constArr[Key(9)]);
    ASSERT_EQ(constArr.data(), &constArr[Key(0)]);
}

}  // anonymous namespace
}  // namespace dawn
