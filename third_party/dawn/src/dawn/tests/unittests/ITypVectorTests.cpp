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

#include <utility>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_vector.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class ITypVectorTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, uint32_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;

    using Vector = ityp::vector<Key, Val>;
};

// Test creation and initialization of the vector.
TEST_F(ITypVectorTest, Creation) {
    // Default constructor initializes to 0
    {
        Vector vec;
        ASSERT_EQ(vec.size(), Key(0));
    }

    // Size constructor initializes contents to 0
    {
        Vector vec(Key(10));
        ASSERT_EQ(vec.size(), Key(10));

        for (Key i(0); i < Key(10); ++i) {
            ASSERT_EQ(vec[i], Val(0));
        }
    }

    // Size and initial value constructor initializes contents to the inital value
    {
        Vector vec(Key(10), Val(7));
        ASSERT_EQ(vec.size(), Key(10));

        for (Key i(0); i < Key(10); ++i) {
            ASSERT_EQ(vec[i], Val(7));
        }
    }

    // Initializer list constructor
    {
        Vector vec = {Val(2), Val(8), Val(1)};
        ASSERT_EQ(vec.size(), Key(3));
        ASSERT_EQ(vec[Key(0)], Val(2));
        ASSERT_EQ(vec[Key(1)], Val(8));
        ASSERT_EQ(vec[Key(2)], Val(1));
    }
}

// Test copy construction/assignment
TEST_F(ITypVectorTest, CopyConstructAssign) {
    // Test the copy constructor
    {
        Vector rhs = {Val(2), Val(8), Val(1)};

        Vector vec(rhs);
        ASSERT_EQ(vec.size(), Key(3));
        ASSERT_EQ(vec[Key(0)], Val(2));
        ASSERT_EQ(vec[Key(1)], Val(8));
        ASSERT_EQ(vec[Key(2)], Val(1));

        ASSERT_EQ(rhs.size(), Key(3));
        ASSERT_EQ(rhs[Key(0)], Val(2));
        ASSERT_EQ(rhs[Key(1)], Val(8));
        ASSERT_EQ(rhs[Key(2)], Val(1));
    }

    // Test the copy assignment
    {
        Vector rhs = {Val(2), Val(8), Val(1)};

        Vector vec = rhs;
        ASSERT_EQ(vec.size(), Key(3));
        ASSERT_EQ(vec[Key(0)], Val(2));
        ASSERT_EQ(vec[Key(1)], Val(8));
        ASSERT_EQ(vec[Key(2)], Val(1));

        ASSERT_EQ(rhs.size(), Key(3));
        ASSERT_EQ(rhs[Key(0)], Val(2));
        ASSERT_EQ(rhs[Key(1)], Val(8));
        ASSERT_EQ(rhs[Key(2)], Val(1));
    }
}

// Test move construction/assignment
TEST_F(ITypVectorTest, MoveConstructAssign) {
    // Test the move constructor
    {
        Vector rhs = {Val(2), Val(8), Val(1)};

        Vector vec(std::move(rhs));
        ASSERT_EQ(vec.size(), Key(3));
        ASSERT_EQ(vec[Key(0)], Val(2));
        ASSERT_EQ(vec[Key(1)], Val(8));
        ASSERT_EQ(vec[Key(2)], Val(1));
    }

    // Test the move assignment
    {
        Vector rhs = {Val(2), Val(8), Val(1)};

        Vector vec = std::move(rhs);
        ASSERT_EQ(vec.size(), Key(3));
        ASSERT_EQ(vec[Key(0)], Val(2));
        ASSERT_EQ(vec[Key(1)], Val(8));
        ASSERT_EQ(vec[Key(2)], Val(1));
    }
}

// Test that values can be set at an index and retrieved from the same index.
TEST_F(ITypVectorTest, Indexing) {
    Vector vec(Key(10));
    {
        vec[Key(2)] = Val(5);
        vec[Key(1)] = Val(9);
        vec[Key(9)] = Val(2);

        ASSERT_EQ(vec[Key(2)], Val(5));
        ASSERT_EQ(vec[Key(1)], Val(9));
        ASSERT_EQ(vec[Key(9)], Val(2));
    }
    {
        vec.at(Key(4)) = Val(5);
        vec.at(Key(3)) = Val(8);
        vec.at(Key(1)) = Val(7);

        ASSERT_EQ(vec.at(Key(4)), Val(5));
        ASSERT_EQ(vec.at(Key(3)), Val(8));
        ASSERT_EQ(vec.at(Key(1)), Val(7));
    }
}

// Test that the vector can be iterated in order with a range-based for loop
TEST_F(ITypVectorTest, RangeBasedIteration) {
    Vector vec(Key(10));

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
    Vector vec(Key(10));

    // non-const versions
    ASSERT_EQ(&vec.front(), &vec[Key(0)]);
    ASSERT_EQ(&vec.back(), &vec[Key(9)]);
    ASSERT_EQ(vec.data(), &vec[Key(0)]);

    // const versions
    const Vector& constVec = vec;
    ASSERT_EQ(&constVec.front(), &constVec[Key(0)]);
    ASSERT_EQ(&constVec.back(), &constVec[Key(9)]);
    ASSERT_EQ(constVec.data(), &constVec[Key(0)]);
}

}  // anonymous namespace
}  // namespace dawn
