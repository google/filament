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

TEST_F(WGSLParserTest, LHSExpression_NoPrefix) {
    auto p = parser("a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, LHSExpression_NoMatch) {
    auto p = parser("123");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
}

TEST_F(WGSLParserTest, LHSExpression_And) {
    auto p = parser("&a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, LHSExpression_Star) {
    auto p = parser("*a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, LHSExpression_InvalidCoreLHSExpr) {
    auto p = parser("*123");
    auto e = p->lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:2: missing expression");
}

TEST_F(WGSLParserTest, LHSExpression_Multiple) {
    auto p = parser("*&********&&&&&&*a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);

    std::vector<core::UnaryOp> results = {
        core::UnaryOp::kIndirection, core::UnaryOp::kAddressOf,   core::UnaryOp::kIndirection,
        core::UnaryOp::kIndirection, core::UnaryOp::kIndirection, core::UnaryOp::kIndirection,
        core::UnaryOp::kIndirection, core::UnaryOp::kIndirection, core::UnaryOp::kIndirection,
        core::UnaryOp::kIndirection, core::UnaryOp::kAddressOf,   core::UnaryOp::kAddressOf,
        core::UnaryOp::kAddressOf,   core::UnaryOp::kAddressOf,   core::UnaryOp::kAddressOf,
        core::UnaryOp::kAddressOf,   core::UnaryOp::kIndirection};

    auto* expr = e.value;
    for (auto op : results) {
        ASSERT_TRUE(expr->Is<ast::UnaryOpExpression>());

        auto* u = expr->As<ast::UnaryOpExpression>();
        EXPECT_EQ(u->op, op);

        expr = u->expr;
    }

    EXPECT_TRUE(expr->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, LHSExpression_PostfixExpression_Array) {
    auto p = parser("*a[0]");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kIndirection);

    ASSERT_TRUE(u->expr->Is<ast::IndexAccessorExpression>());

    auto* access = u->expr->As<ast::IndexAccessorExpression>();
    ASSERT_TRUE(access->object->Is<ast::IdentifierExpression>());

    auto* obj = access->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(obj->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(access->index->Is<ast::IntLiteralExpression>());
    auto* idx = access->index->As<ast::IntLiteralExpression>();
    EXPECT_EQ(0, idx->value);
}

TEST_F(WGSLParserTest, LHSExpression_PostfixExpression) {
    auto p = parser("*a.foo");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, core::UnaryOp::kIndirection);

    ASSERT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());

    auto* access = u->expr->As<ast::MemberAccessorExpression>();
    ASSERT_TRUE(access->object->Is<ast::IdentifierExpression>());

    auto* struct_ident = access->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(struct_ident->identifier->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(access->member->symbol, p->builder().Symbols().Get("foo"));
}

TEST_F(WGSLParserTest, LHSExpression_InvalidPostfixExpression) {
    auto p = parser("*a.if");
    auto e = p->lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:4: expected identifier for member accessor");
}

}  // namespace
}  // namespace tint::wgsl::reader
