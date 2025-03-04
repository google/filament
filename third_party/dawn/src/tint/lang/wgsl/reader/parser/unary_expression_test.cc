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

#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, UnaryExpression_Postix) {
    auto p = parser("a[2]");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IndexAccessorExpression>());
    auto* idx = e->As<ast::IndexAccessorExpression>();
    ASSERT_TRUE(idx->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = idx->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(idx->index->Is<ast::IntLiteralExpression>());
    ASSERT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 2);
    ASSERT_EQ(idx->index->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, UnaryExpression_Minus) {
    auto p = parser("- 1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, core::UnaryOp::kNegation);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, UnaryExpression_AddressOf) {
    auto p = parser("&x");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, UnaryExpression_Dereference) {
    auto p = parser("*x");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, UnaryExpression_AddressOf_Precedence) {
    auto p = parser("&x.y");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());
}

TEST_F(WGSLParserTest, UnaryExpression_Dereference_Precedence) {
    auto p = parser("*x.y");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());
}

TEST_F(WGSLParserTest, UnaryExpression_Minus_InvalidRHS) {
    auto p = parser("-if(a) {}");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse right side of - expression");
}

TEST_F(WGSLParserTest, UnaryExpression_Bang) {
    auto p = parser("!1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, core::UnaryOp::kNot);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, UnaryExpression_Bang_InvalidRHS) {
    auto p = parser("!if (a) {}");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse right side of ! expression");
}

TEST_F(WGSLParserTest, UnaryExpression_Tilde) {
    auto p = parser("~1");
    auto e = p->unary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    ASSERT_EQ(u->op, core::UnaryOp::kComplement);

    ASSERT_TRUE(u->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(u->expr->As<ast::IntLiteralExpression>()->value, 1);
    ASSERT_EQ(u->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, UnaryExpression_PrefixPlusPlus) {
    auto p = parser("++a");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(),
              "1:1: prefix increment and decrement operators are reserved for a "
              "future WGSL version");
}

TEST_F(WGSLParserTest, UnaryExpression_PrefixMinusMinus) {
    auto p = parser("--a");
    auto e = p->unary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(),
              "1:1: prefix increment and decrement operators are reserved for a "
              "future WGSL version");
}

}  // namespace
}  // namespace tint::wgsl::reader
