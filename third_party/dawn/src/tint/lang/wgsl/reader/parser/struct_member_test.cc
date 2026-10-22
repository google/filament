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

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, StructMember_Parses) {
    auto p = parser("a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 0u);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 1u}, {1u, 2u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 5u}, {1u, 8u}}));
}

TEST_F(WGSLParserTest, StructMember_ParsesWithAlignAttribute) {
    auto p = parser("@align(2) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 1u);
    EXPECT_TRUE(m->attributes[0]->Is<ast::StructMemberAlignAttribute>());

    auto* attr = m->attributes[0]->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(attr->expr->Is<ast::IntLiteralExpression>());
    auto* expr = attr->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 2);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 11u}, {1u, 12u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 15u}, {1u, 18u}}));
}

TEST_F(WGSLParserTest, StructMember_ParsesWithSizeAttribute) {
    auto p = parser("@size(2) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 1u);
    ASSERT_TRUE(m->attributes[0]->Is<ast::StructMemberSizeAttribute>());
    auto* s = m->attributes[0]->As<ast::StructMemberSizeAttribute>();

    ASSERT_TRUE(s->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(s->expr->As<ast::IntLiteralExpression>()->value, 2u);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 10u}, {1u, 11u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 14u}, {1u, 17u}}));
}

TEST_F(WGSLParserTest, StructMember_ParsesWithMultipleattributes) {
    auto p = parser(R"(@size(2)
@align(4) a : i32,)");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 2u);
    ASSERT_TRUE(m->attributes[0]->Is<ast::StructMemberSizeAttribute>());
    auto* size_attr = m->attributes[0]->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(size_attr->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(size_attr->expr->As<ast::IntLiteralExpression>()->value, 2u);

    ASSERT_TRUE(m->attributes[1]->Is<ast::StructMemberAlignAttribute>());
    auto* attr = m->attributes[1]->As<ast::StructMemberAlignAttribute>();

    ASSERT_TRUE(attr->expr->Is<ast::IntLiteralExpression>());
    auto* expr = attr->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(m->source.range, (Source::Range{{2u, 11u}, {2u, 12u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{2u, 15u}, {2u, 18u}}));
}

TEST_F(WGSLParserTest, StructMember_InvalidAttribute) {
    auto p = parser("@size(if) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_TRUE(m.errored);
    ASSERT_EQ(m.value, nullptr);

    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected expression for size");
}

}  // namespace
}  // namespace tint::wgsl::reader
