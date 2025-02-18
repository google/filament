// Copyright 2024 The langsvr Authors
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

#include "langsvr/one_of.h"

#include "gmock/gmock.h"

namespace langsvr {
namespace {

TEST(OneOfTest, CtorNoArgs) {
    OneOf<int, std::string, float> oneof;
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<float>());
    oneof.Visit([&](auto&) { FAIL() << "should not be called"; });
}

TEST(OneOfTest, CopyCtorWithValue) {
    std::string val{"hello"};
    OneOf<int, std::string, float> oneof{val};
    ASSERT_TRUE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<float>());
    EXPECT_EQ(*oneof.Get<std::string>(), std::string("hello"));

    std::variant<int, std::string, float> visit_val;
    auto res = oneof.Visit([&](auto& v) {
        visit_val = v;
        return 42;
    });
    EXPECT_EQ(res, 42);
    ASSERT_TRUE(std::holds_alternative<std::string>(visit_val));
    EXPECT_EQ(std::get<std::string>(visit_val), std::string("hello"));
}

TEST(OneOfTest, MoveCtorWithValue) {
    std::string val{"hello"};
    OneOf<int, std::string, float> oneof{std::move(val)};
    ASSERT_TRUE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<float>());
    EXPECT_EQ(*oneof.Get<std::string>(), std::string("hello"));

    std::variant<int, std::string, float> visit_val;
    auto res = oneof.Visit([&](auto& v) {
        visit_val = v;
        return 42;
    });
    EXPECT_EQ(res, 42);
    ASSERT_TRUE(std::holds_alternative<std::string>(visit_val));
    EXPECT_EQ(std::get<std::string>(visit_val), std::string("hello"));
}

TEST(OneOfTest, CopyCtorWithOneOf) {
    OneOf<int, std::string, float> other{std::string("hello")};
    OneOf<int, std::string, float> oneof{other};
    ASSERT_TRUE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<float>());
    EXPECT_EQ(*oneof.Get<std::string>(), std::string("hello"));

    std::variant<int, std::string, float> visit_val;
    auto res = oneof.Visit([&](auto& v) {
        visit_val = v;
        return 42;
    });
    EXPECT_EQ(res, 42);
    ASSERT_TRUE(std::holds_alternative<std::string>(visit_val));
    EXPECT_EQ(std::get<std::string>(visit_val), std::string("hello"));
}

TEST(OneOfTest, MoveCtorWithOneOf) {
    OneOf<int, std::string, float> other{std::string("hello")};
    OneOf<int, std::string, float> oneof{std::move(other)};
    ASSERT_TRUE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<float>());
    EXPECT_EQ(*oneof.Get<std::string>(), std::string("hello"));

    std::variant<int, std::string, float> visit_val;
    auto res = oneof.Visit([&](auto& v) {
        visit_val = v;
        return 42;
    });
    EXPECT_EQ(res, 42);
    ASSERT_TRUE(std::holds_alternative<std::string>(visit_val));
    EXPECT_EQ(std::get<std::string>(visit_val), std::string("hello"));
}

TEST(OneOfTest, Reset) {
    OneOf<int, std::string, float> oneof{std::string("hello")};
    oneof.Reset();
    EXPECT_FALSE(oneof.Is<std::string>());
    EXPECT_FALSE(oneof.Is<int>());
    EXPECT_FALSE(oneof.Is<float>());
    oneof.Visit([&](auto&) { FAIL() << "should not be called"; });
}

}  // namespace
}  // namespace langsvr::lsp
