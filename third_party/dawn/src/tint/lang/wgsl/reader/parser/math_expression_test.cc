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

TEST_F(WGSLParserTest, MathExpression_Parses_Multiplicative) {
    auto p = parser("a * b");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(WGSLParserTest, MathExpression_Parses_Mixed_MultiplicativeStart) {
    auto p = parser("a * b + c");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    // lhs: a * b
    // op: +
    // rhs: c
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAdd, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("c"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: a
    // op: *
    // rhs: b
    rel = rel->lhs->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(WGSLParserTest, MathExpression_Parses_Additive) {
    auto p = parser("a + b");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAdd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(WGSLParserTest, MathExpression_Parses_Mixed_AdditiveStart) {
    auto p = parser("a + b * c");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    // lhs: a
    // op: +
    // rhs: b * c
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAdd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BinaryExpression>());
    // lhs: b
    // op: *
    // rhs: c
    rel = rel->rhs->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kMultiply, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("c"));
}

TEST_F(WGSLParserTest, MathExpression_NoMatch_ReturnLHS) {
    auto p = parser("a if");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_EQ(lhs.value, e.value);
}

TEST_F(WGSLParserTest, MathExpression_InvalidRHS) {
    auto p = parser("a * if");
    auto lhs = p->unary_expression();
    auto e = p->expect_math_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ("1:5: unable to parse right side of * expression", p->error());
}

}  // namespace
}  // namespace tint::wgsl::reader
