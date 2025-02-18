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

TEST_F(WGSLParserTest, Attribute_Size) {
    auto p = parser("size(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Size_Expression) {
    auto p = parser("size(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::BinaryExpression>());
    auto* expr = o->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_Size_TrailingComma) {
    auto p = parser("size(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Size_MissingLeftParen) {
    auto p = parser("size 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected '(' for size attribute");
}

TEST_F(WGSLParserTest, Attribute_Size_MissingRightParen) {
    auto p = parser("size(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected ')' for size attribute");
}

TEST_F(WGSLParserTest, Attribute_Size_MissingValue) {
    auto p = parser("size()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: size expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Size_MissingInvalid) {
    auto p = parser("size(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected expression for size");
}

TEST_F(WGSLParserTest, Attribute_Align) {
    auto p = parser("align(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error());

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Align_Expression) {
    auto p = parser("align(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(attr.value, nullptr);

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::BinaryExpression>());
    auto* expr = o->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_Align_TrailingComma) {
    auto p = parser("align(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());

    auto* expr = o->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Align_MissingLeftParen) {
    auto p = parser("align 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for align attribute");
}

TEST_F(WGSLParserTest, Attribute_Align_MissingRightParen) {
    auto p = parser("align(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for align attribute");
}

TEST_F(WGSLParserTest, Attribute_Align_MissingValue) {
    auto p = parser("align()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: align expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Align_ExpressionInvalid) {
    auto p = parser("align(4 + 5 << 6)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());

    EXPECT_EQ(p->builder().Diagnostics().Str(),
              R"(test.wgsl:1:9 error: mixing '+' and '<<' requires parenthesis
align(4 + 5 << 6)
        ^^^^^^
)");
}

TEST_F(WGSLParserTest, Attribute_BlendSrc) {
    auto p = parser("blend_src(1)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::BlendSrcAttribute>());

    auto* o = member_attr->As<ast::BlendSrcAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 1);
}

}  // namespace
}  // namespace tint::wgsl::reader
