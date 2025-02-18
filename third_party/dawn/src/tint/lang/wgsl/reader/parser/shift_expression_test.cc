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

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_Parses_ShiftLeft) {
    auto p = parser("a << true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 10u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kShiftLeft, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_Parses_ShiftRight) {
    auto p = parser("a >> true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 10u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kShiftRight, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_Parses_Additive) {
    auto p = parser("a + b");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
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

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_Parses_Multiplicative) {
    auto p = parser("a * b");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
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

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_InvalidSpaceLeft) {
    auto p = parser("a < < true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    EXPECT_FALSE(e.value->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_InvalidSpaceRight) {
    auto p = parser("a > > true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    EXPECT_FALSE(e.value->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_InvalidRHS) {
    auto p = parser("a << if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: unable to parse right side of << expression");
}

TEST_F(WGSLParserTest, ShiftExpression_PostUnary_NoOr_ReturnsLHS) {
    auto p = parser("a true");
    auto lhs = p->unary_expression();
    auto e = p->expect_shift_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(lhs.value, e.value);
}

TEST_F(WGSLParserTest, ShiftExpression_Parses) {
    auto p = parser("a << true");
    auto e = p->shift_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kShiftLeft, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, ShiftExpression_Invalid_Unary) {
    auto p = parser("if >> true");
    auto e = p->shift_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e.value, nullptr);
}

TEST_F(WGSLParserTest, ShiftExpression_Inavlid_ShiftExpressionPostUnary) {
    auto p = parser("a * if (a) {}");
    auto e = p->shift_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    ASSERT_EQ(e.value, nullptr);

    EXPECT_EQ(p->error(), "1:5: unable to parse right side of * expression");
}

}  // namespace
}  // namespace tint::wgsl::reader
