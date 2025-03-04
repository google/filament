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

TEST_F(WGSLParserTest, IncrementDecrementStmt_Increment) {
    auto p = parser("a++");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* i = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(i, nullptr);
    ASSERT_NE(i->lhs, nullptr);

    ASSERT_TRUE(i->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = i->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    EXPECT_TRUE(i->increment);
}

TEST_F(WGSLParserTest, IncrementDecrementStmt_Decrement) {
    auto p = parser("a--");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* i = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(i, nullptr);
    ASSERT_NE(i->lhs, nullptr);

    ASSERT_TRUE(i->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = i->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    EXPECT_FALSE(i->increment);
}

TEST_F(WGSLParserTest, IncrementDecrementStmt_Parenthesized) {
    auto p = parser("(a)++");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* i = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(i, nullptr);
    ASSERT_NE(i->lhs, nullptr);

    ASSERT_TRUE(i->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = i->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    EXPECT_TRUE(i->increment);
}

TEST_F(WGSLParserTest, IncrementDecrementStmt_ToMember) {
    auto p = parser("a.b.c[2].d++");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* i = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(i, nullptr);
    ASSERT_NE(i->lhs, nullptr);
    EXPECT_TRUE(i->increment);

    ASSERT_TRUE(i->lhs->Is<ast::MemberAccessorExpression>());
    auto* mem = i->lhs->As<ast::MemberAccessorExpression>();

    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("d"));

    ASSERT_TRUE(mem->object->Is<ast::IndexAccessorExpression>());
    auto* idx = mem->object->As<ast::IndexAccessorExpression>();

    ASSERT_NE(idx->index, nullptr);
    ASSERT_TRUE(idx->index->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(idx->object->Is<ast::MemberAccessorExpression>());
    mem = idx->object->As<ast::MemberAccessorExpression>();
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("c"));

    ASSERT_TRUE(mem->object->Is<ast::MemberAccessorExpression>());
    mem = mem->object->As<ast::MemberAccessorExpression>();

    ASSERT_TRUE(mem->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = mem->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(WGSLParserTest, IncrementDecrementStmt_InvalidLHS) {
    auto p = parser("{}++");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e.value, nullptr);
}

}  // namespace
}  // namespace tint::wgsl::reader
