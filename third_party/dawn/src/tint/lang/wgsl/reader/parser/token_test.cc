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

#include "src/tint/lang/wgsl/reader/parser/token.h"

#include <limits>

#include "gmock/gmock.h"

namespace tint::wgsl::reader {
namespace {

using ::testing::EndsWith;
using ::testing::Not;
using ::testing::StartsWith;

using TokenTest = testing::Test;

TEST_F(TokenTest, ReturnsF64) {
    Token t1(Token::Type::kFloatLiteral_F, Source{}, -2.345);
    EXPECT_EQ(t1.to_f64(), -2.345);

    Token t2(Token::Type::kFloatLiteral_F, Source{}, 2.345);
    EXPECT_EQ(t2.to_f64(), 2.345);
}

TEST_F(TokenTest, ReturnsI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{}, static_cast<int64_t>(-2345));
    EXPECT_EQ(t1.to_i64(), -2345);

    Token t2(Token::Type::kIntLiteral_I, Source{}, static_cast<int64_t>(2345));
    EXPECT_EQ(t2.to_i64(), 2345);
}

TEST_F(TokenTest, HandlesMaxI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{},
             static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<int32_t>::max());
}

TEST_F(TokenTest, HandlesMinI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{},
             static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<int32_t>::min());
}

TEST_F(TokenTest, ReturnsU32) {
    Token t2(Token::Type::kIntLiteral_U, Source{}, static_cast<int64_t>(2345u));
    EXPECT_EQ(t2.to_i64(), 2345u);
}

TEST_F(TokenTest, ReturnsMaxU32) {
    Token t1(Token::Type::kIntLiteral_U, Source{},
             static_cast<int64_t>(std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<uint32_t>::max());
}

TEST_F(TokenTest, Source) {
    Source src;
    src.range.begin = Source::Location{3, 9};
    src.range.end = Source::Location{4, 3};

    Token t(Token::Type::kIntLiteral, src);
    EXPECT_EQ(t.source().range.begin.line, 3u);
    EXPECT_EQ(t.source().range.begin.column, 9u);
    EXPECT_EQ(t.source().range.end.line, 4u);
    EXPECT_EQ(t.source().range.end.column, 3u);
}

TEST_F(TokenTest, ToStr) {
    double d = 123.0;
    int64_t i = 123;
    EXPECT_THAT(Token(Token::Type::kFloatLiteral, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral, Source{}, d).to_str(), Not(EndsWith("f")));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_F, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_F, Source{}, d).to_str(), EndsWith("f"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_H, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_H, Source{}, d).to_str(), EndsWith("h"));
    EXPECT_EQ(Token(Token::Type::kIntLiteral, Source{}, i).to_str(), "123");
    EXPECT_EQ(Token(Token::Type::kIntLiteral_I, Source{}, i).to_str(), "123i");
    EXPECT_EQ(Token(Token::Type::kIntLiteral_U, Source{}, i).to_str(), "123u");
    EXPECT_EQ(Token(Token::Type::kIdentifier, Source{}, "blah").to_str(), "blah");
    EXPECT_EQ(Token(Token::Type::kError, Source{}, "blah").to_str(), "blah");
}

}  // namespace
}  // namespace tint::wgsl::reader
