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

TEST_F(WGSLParserTest, BitwiseExpr_NoOp) {
    auto p = parser("a true");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e.value, nullptr);
}

TEST_F(WGSLParserTest, BitwiseExpr_Or_Parses) {
    auto p = parser("a | true");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 9u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kOr, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_Or_Parses_Multiple) {
    auto p = parser("a | true | b");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    // lhs: (a | true)
    // rhs: b

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kOr, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());

    // lhs: a
    // rhs: true
    rel = rel->lhs->As<ast::BinaryExpression>();

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_Or_InvalidRHS) {
    auto p = parser("true | if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unable to parse right side of | expression");
}

TEST_F(WGSLParserTest, BitwiseExpr_Xor_Parses) {
    auto p = parser("a ^ true");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 9u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kXor, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_Xor_Parses_Multiple) {
    auto p = parser("a ^ true ^ b");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    // lhs: (a ^ true)
    // rhs: b
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kXor, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("b"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());

    // lhs: a
    // rhs: true
    rel = rel->lhs->As<ast::BinaryExpression>();

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_Xor_InvalidRHS) {
    auto p = parser("true ^ if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unable to parse right side of ^ expression");
}

TEST_F(WGSLParserTest, BitwiseExpr_And_Parses) {
    auto p = parser("a & true");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 9u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAnd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Register("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_And_Parses_Multiple) {
    auto p = parser("a & true & b");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    // lhs: (a & true)
    // rhs: b
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAnd, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Register("b"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());

    // lhs: a
    // rhs: true
    rel = rel->lhs->As<ast::BinaryExpression>();

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident_expr = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Register("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, BitwiseExpr_And_Parses_AndAnd) {
    auto p = parser("a & true &&b");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    // bitwise_expression_post_unary_expression returns before parsing '&&'

    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    // lhs: a
    // rhs: true
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(core::BinaryOp::kAnd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
}

TEST_F(WGSLParserTest, BitwiseExpr_And_InvalidRHS) {
    auto p = parser("true & if (a) {}");
    auto lhs = p->unary_expression();
    auto e = p->bitwise_expression_post_unary_expression(lhs.value, lhs->source);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unable to parse right side of & expression");
}

}  // namespace
}  // namespace tint::wgsl::reader
