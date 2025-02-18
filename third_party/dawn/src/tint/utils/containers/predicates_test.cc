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

#include "src/tint/utils/containers/predicates.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(PredicatesTest, Eq) {
    auto pred = Eq(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, Ne) {
    auto pred = Ne(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Gt) {
    auto pred = Gt(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Lt) {
    auto pred = Lt(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, Ge) {
    auto pred = Ge(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Le) {
    auto pred = Le(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, IsNull) {
    int i = 1;
    EXPECT_TRUE(IsNull(nullptr));
    EXPECT_FALSE(IsNull(&i));
}

}  // namespace
}  // namespace tint
