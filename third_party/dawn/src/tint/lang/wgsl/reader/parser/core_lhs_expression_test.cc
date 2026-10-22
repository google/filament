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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, CoreLHS_NoMatch) {
    auto p = parser("123");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
}

TEST_F(WGSLParserTest, CoreLHS_Ident) {
    auto p = parser("identifier");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());

    auto* ident_expr = e->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("identifier"));
}

TEST_F(WGSLParserTest, CoreLHS_ParenStmt) {
    auto p = parser("(a)");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());

    auto* ident_expr = e->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

TEST_F(WGSLParserTest, CoreLHS_MissingRightParen) {
    auto p = parser("(a");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected ')'");
}

TEST_F(WGSLParserTest, CoreLHS_InvalidLHSExpression) {
    auto p = parser("(if (a() {})");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: invalid expression");
}

TEST_F(WGSLParserTest, CoreLHS_MissingLHSExpression) {
    auto p = parser("()");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: invalid expression");
}

TEST_F(WGSLParserTest, CoreLHS_Invalid) {
    auto p = parser("1234");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
}

}  // namespace
}  // namespace tint::wgsl::reader
