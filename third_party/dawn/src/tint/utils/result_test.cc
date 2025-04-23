// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/utils/result.h"

#include <string>

#include "gmock/gmock.h"

namespace tint {
namespace {

struct S {
    int value;
};
static inline bool operator==(const S& a, const S& b) {
    return a.value == b.value;
}

TEST(ResultTest, SuccessInt) {
    auto r = Result<int>(123);
    ASSERT_EQ(r, Success);
    EXPECT_EQ(r.Get(), 123);
}

TEST(ResultTest, SuccessStruct) {
    auto r = Result<S>({123});
    ASSERT_EQ(r, Success);
    EXPECT_EQ(r->value, 123);
    EXPECT_EQ(r, S{123});
}

TEST(ResultTest, Failure) {
    auto r = Result<int>(Failure{});
    EXPECT_NE(r, Success);
}

TEST(ResultTest, CustomFailure) {
    auto r = Result<int, std::string>("oh noes!");
    EXPECT_NE(r, Success);
    EXPECT_EQ(r.Failure(), "oh noes!");
}

TEST(ResultTest, ValueCast) {
    struct X {};
    struct Y : X {};

    Y* y = nullptr;
    auto r_y = Result<Y*>{y};
    auto r_x = Result<X*>{r_y};

    (void)r_x;
    (void)r_y;
}

}  // namespace
}  // namespace tint
