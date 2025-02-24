// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/containers/unique_vector.h"

#include <vector>

#include "src/tint/utils/containers/reverse.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(UniqueVectorTest, Empty) {
    UniqueVector<int, 4> unique_vec;
    ASSERT_EQ(unique_vec.Length(), 0u);
    EXPECT_EQ(unique_vec.IsEmpty(), true);
    EXPECT_EQ(unique_vec.begin(), unique_vec.end());
}

TEST(UniqueVectorTest, MoveConstructor) {
    UniqueVector<int, 4> unique_vec(std::vector<int>{0, 3, 2, 1, 2});
    ASSERT_EQ(unique_vec.Length(), 4u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 3);
    EXPECT_EQ(unique_vec[2], 2);
    EXPECT_EQ(unique_vec[3], 1);
}

TEST(UniqueVectorTest, AddUnique) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(2);
    ASSERT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    int i = 0;
    for (auto n : unique_vec) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_EQ(unique_vec[2], 2);
}

TEST(UniqueVectorTest, AddDuplicates) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(1);
    unique_vec.Add(2);
    ASSERT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    int i = 0;
    for (auto n : unique_vec) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_EQ(unique_vec[2], 2);
}

TEST(UniqueVectorTest, Erase) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(3);
    unique_vec.Add(2);
    unique_vec.Add(5);
    unique_vec.Add(1);
    unique_vec.Add(6);
    EXPECT_EQ(unique_vec.Length(), 6u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);

    unique_vec.Erase(2, 2);

    ASSERT_EQ(unique_vec.Length(), 4u);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 3);
    EXPECT_EQ(unique_vec[2], 1);
    EXPECT_EQ(unique_vec[3], 6);
    EXPECT_TRUE(unique_vec.Contains(0));
    EXPECT_TRUE(unique_vec.Contains(3));
    EXPECT_FALSE(unique_vec.Contains(2));
    EXPECT_FALSE(unique_vec.Contains(5));
    EXPECT_TRUE(unique_vec.Contains(1));
    EXPECT_TRUE(unique_vec.Contains(6));
    EXPECT_EQ(unique_vec.IsEmpty(), false);

    unique_vec.Erase(1);

    ASSERT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_EQ(unique_vec[2], 6);
    EXPECT_TRUE(unique_vec.Contains(0));
    EXPECT_FALSE(unique_vec.Contains(3));
    EXPECT_FALSE(unique_vec.Contains(2));
    EXPECT_FALSE(unique_vec.Contains(5));
    EXPECT_TRUE(unique_vec.Contains(1));
    EXPECT_TRUE(unique_vec.Contains(6));
    EXPECT_EQ(unique_vec.IsEmpty(), false);

    unique_vec.Erase(2);

    ASSERT_EQ(unique_vec.Length(), 2u);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_TRUE(unique_vec.Contains(0));
    EXPECT_FALSE(unique_vec.Contains(3));
    EXPECT_FALSE(unique_vec.Contains(2));
    EXPECT_FALSE(unique_vec.Contains(5));
    EXPECT_TRUE(unique_vec.Contains(1));
    EXPECT_FALSE(unique_vec.Contains(6));
    EXPECT_EQ(unique_vec.IsEmpty(), false);

    unique_vec.Erase(0, 2);

    ASSERT_EQ(unique_vec.Length(), 0u);
    EXPECT_FALSE(unique_vec.Contains(0));
    EXPECT_FALSE(unique_vec.Contains(3));
    EXPECT_FALSE(unique_vec.Contains(2));
    EXPECT_FALSE(unique_vec.Contains(5));
    EXPECT_FALSE(unique_vec.Contains(1));
    EXPECT_FALSE(unique_vec.Contains(6));
    EXPECT_EQ(unique_vec.IsEmpty(), true);

    unique_vec.Add(6);
    unique_vec.Add(0);
    unique_vec.Add(2);

    ASSERT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec[0], 6);
    EXPECT_EQ(unique_vec[1], 0);
    EXPECT_EQ(unique_vec[2], 2);
    EXPECT_TRUE(unique_vec.Contains(0));
    EXPECT_FALSE(unique_vec.Contains(3));
    EXPECT_TRUE(unique_vec.Contains(2));
    EXPECT_FALSE(unique_vec.Contains(5));
    EXPECT_FALSE(unique_vec.Contains(1));
    EXPECT_TRUE(unique_vec.Contains(6));
    EXPECT_EQ(unique_vec.IsEmpty(), false);
}

TEST(UniqueVectorTest, AsVector) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(1);
    unique_vec.Add(2);

    VectorRef<int> ref = unique_vec;
    EXPECT_EQ(ref.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    int i = 0;
    for (auto n : ref) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
}

TEST(UniqueVectorTest, PopBack) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(2);
    unique_vec.Add(1);

    EXPECT_EQ(unique_vec.Pop(), 1);
    ASSERT_EQ(unique_vec.Length(), 2u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 2);

    EXPECT_EQ(unique_vec.Pop(), 2);
    ASSERT_EQ(unique_vec.Length(), 1u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    unique_vec.Add(1);

    ASSERT_EQ(unique_vec.Length(), 2u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);

    EXPECT_EQ(unique_vec.Pop(), 1);
    ASSERT_EQ(unique_vec.Length(), 1u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    EXPECT_EQ(unique_vec.Pop(), 0);
    EXPECT_EQ(unique_vec.Length(), 0u);
    EXPECT_EQ(unique_vec.IsEmpty(), true);
}

TEST(UniqueVectorTest, Data) {
    UniqueVector<int, 4> unique_vec;
    EXPECT_EQ(unique_vec.Data(), nullptr);

    unique_vec.Add(42);
    EXPECT_EQ(unique_vec.Data(), &unique_vec[0]);
    EXPECT_EQ(*unique_vec.Data(), 42);
}

}  // namespace
}  // namespace tint
