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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, Empty) {
    auto p = parser("");
    ASSERT_TRUE(p->Parse()) << p->error();
}

TEST_F(WGSLParserTest, Parses) {
    auto p = parser(R"(
@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(.4, .2, .3, 1);
}
)");
    ASSERT_TRUE(p->Parse()) << p->error();

    Program program = p->program();
    ASSERT_EQ(1u, program.AST().Functions().Length());
}

TEST_F(WGSLParserTest, Parses_ExtraCommas) {
    auto p = parser(R"(
@group(0) @binding(0) var<storage,> x: i32;
@group(0) @binding(1) var<storage,read_write,> x: i32;
)");
    ASSERT_TRUE(p->Parse()) << p->error();
    Program program = p->program();
    ASSERT_EQ(2u, program.AST().GlobalVariables().Length());
}

TEST_F(WGSLParserTest, Parses_ExtraSemicolons) {
    auto p = parser(R"(
;
struct S {
  a : f32,
};;
;
fn foo() -> S {
  ;
  return S();;;
  ;
};;
;
)");
    ASSERT_TRUE(p->Parse()) << p->error();

    Program program = p->program();
    ASSERT_EQ(1u, program.AST().Functions().Length());
    ASSERT_EQ(1u, program.AST().TypeDecls().Length());
}

TEST_F(WGSLParserTest, HandlesError) {
    auto p = parser(R"(
fn main() ->  {  // missing return type
  return;
})");

    ASSERT_FALSE(p->Parse());
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "2:15: unable to determine function return type");
}

TEST_F(WGSLParserTest, HandlesUnexpectedToken) {
    auto p = parser(R"(
fn main() {
}
foobar
)");

    ASSERT_FALSE(p->Parse());
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "4:1: unexpected token");
}

TEST_F(WGSLParserTest, HandlesBadToken_InMiddle) {
    auto p = parser(R"(
fn main() {
  let f = 0x1p10000000000000000000; // Exponent too big for hex float
  return;
})");

    ASSERT_FALSE(p->Parse());
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:11: exponent is too large for hex float");
}

TEST_F(WGSLParserTest, HandlesBadToken_AtModuleScope) {
    auto p = parser(R"(
fn main() {
  return;
}
0x1p10000000000000000000
)");

    ASSERT_FALSE(p->Parse());
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "5:1: exponent is too large for hex float");
}

TEST_F(WGSLParserTest, Comments_TerminatedBlockComment) {
    auto p = parser(R"(
/**
 * Here is my shader.
 *
 * /* I can nest /**/ comments. */
 * // I can nest line comments too.
 **/
@fragment // This is the stage
fn main(/*
no
parameters
*/) -> @location(0) vec4<f32> {
  return/*block_comments_delimit_tokens*/vec4<f32>(.4, .2, .3, 1);
}/* block comments are OK at EOF...*/)");

    ASSERT_TRUE(p->Parse()) << p->error();
    ASSERT_EQ(1u, p->program().AST().Functions().Length());
}

TEST_F(WGSLParserTest, Comments_UnterminatedBlockComment) {
    auto p = parser(R"(
@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(.4, .2, .3, 1);
} /* unterminated block comments are invalid ...)");

    ASSERT_FALSE(p->Parse());
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "5:3: unterminated block comment") << p->error();
}

TEST_F(WGSLParserTest, Peek) {
    auto p = parser("a == if");
    EXPECT_TRUE(p->peek_is(Token::Type::kIdentifier));
    EXPECT_TRUE(p->peek_is(Token::Type::kEqualEqual, 1));
    EXPECT_TRUE(p->peek_is(Token::Type::kIf, 2));
}

TEST_F(WGSLParserTest, Peek_Placeholder) {
    auto p = parser(">> if");
    EXPECT_TRUE(p->peek_is(Token::Type::kShiftRight));
    EXPECT_TRUE(p->peek_is(Token::Type::kIf, 1));
}

TEST_F(WGSLParserTest, Peek_PastPlaceholder) {
    auto p = parser(">= vec2<u32>");
    auto& n = p->next();
    ASSERT_TRUE(n.Is(Token::Type::kGreaterThanEqual));
    EXPECT_TRUE(p->peek_is(Token::Type::kIdentifier))
        << "expected: vec2 got: " << p->peek().to_name();
    EXPECT_TRUE(p->peek_is(Token::Type::kTemplateArgsLeft, 1))
        << "expected: < got: " << p->peek(1).to_name();
}

TEST_F(WGSLParserTest, Peek_MultiplePlaceholder) {
    auto p = parser(">= >= vec2<u32>");
    auto& n = p->next();
    ASSERT_TRUE(n.Is(Token::Type::kGreaterThanEqual));
    EXPECT_TRUE(p->peek_is(Token::Type::kGreaterThanEqual))
        << "expected: >= got: " << p->peek().to_name();
    EXPECT_TRUE(p->peek_is(Token::Type::kIdentifier, 1))
        << "expected: vec2 got: " << p->peek(1).to_name();
    EXPECT_TRUE(p->peek_is(Token::Type::kTemplateArgsLeft, 2))
        << "expected: < got: " << p->peek(2).to_name();
}

TEST_F(WGSLParserTest, Peek_PastEnd) {
    auto p = parser(">");
    EXPECT_TRUE(p->peek_is(Token::Type::kGreaterThan));
    EXPECT_TRUE(p->peek_is(Token::Type::kEOF, 1));
    EXPECT_TRUE(p->peek_is(Token::Type::kEOF, 2));
}

TEST_F(WGSLParserTest, Peek_PastEnd_WalkingPlaceholders) {
    auto p = parser(">= >=");
    auto& n = p->next();
    ASSERT_TRUE(n.Is(Token::Type::kGreaterThanEqual));
    EXPECT_TRUE(p->peek_is(Token::Type::kGreaterThanEqual))
        << "expected: <= got: " << p->peek().to_name();
    EXPECT_TRUE(p->peek_is(Token::Type::kEOF, 1)) << "expected: EOF got: " << p->peek(1).to_name();
}

TEST_F(WGSLParserTest, Peek_AfterSplit) {
    auto p = parser(">= vec2<u32>");
    auto& n = p->next();
    ASSERT_TRUE(n.Is(Token::Type::kGreaterThanEqual));
    EXPECT_TRUE(p->peek_is(Token::Type::kIdentifier))
        << "expected: vec2 got: " << p->peek().to_name();

    p->split_token(Token::Type::kGreaterThan, Token::Type::kEqual);
    ASSERT_TRUE(n.Is(Token::Type::kGreaterThan));
    EXPECT_TRUE(p->peek_is(Token::Type::kEqual)) << "expected: = got: " << p->peek().to_name();
}

}  // namespace
}  // namespace tint::wgsl::reader
