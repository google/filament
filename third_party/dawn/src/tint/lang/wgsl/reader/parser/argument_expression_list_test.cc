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

TEST_F(WGSLParserTest, ArgumentExpressionList_Parses) {
    auto p = parser("(a)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 1u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, ArgumentExpressionList_ParsesEmptyList) {
    auto p = parser("()");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 0u);
}

TEST_F(WGSLParserTest, ArgumentExpressionList_ParsesMultiple) {
    auto p = parser("(a, 33, 1+2)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 3u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
    ASSERT_TRUE(e.value[1]->Is<ast::LiteralExpression>());
    ASSERT_TRUE(e.value[2]->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, ArgumentExpressionList_TrailingComma) {
    auto p = parser("(a, 42,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 2u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
    ASSERT_TRUE(e.value[1]->Is<ast::LiteralExpression>());
}

TEST_F(WGSLParserTest, ArgumentExpressionList_HandlesMissingLeftParen) {
    auto p = parser("a)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: expected '(' for argument list");
}

TEST_F(WGSLParserTest, ArgumentExpressionList_HandlesMissingRightParen) {
    auto p = parser("(a");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:3: expected ',' for argument list");
}

TEST_F(WGSLParserTest, ArgumentExpressionList_HandlesMissingExpression_0) {
    auto p = parser("(,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:2: expected expression for argument list");
}

TEST_F(WGSLParserTest, ArgumentExpressionList_HandlesMissingExpression_1) {
    auto p = parser("(a, ,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:5: expected expression for argument list");
}

TEST_F(WGSLParserTest, ArgumentExpressionList_HandlesInvalidExpression) {
    auto p = parser("(if(a) {})");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:2: expected expression for argument list");
}

}  // namespace
}  // namespace tint::wgsl::reader
