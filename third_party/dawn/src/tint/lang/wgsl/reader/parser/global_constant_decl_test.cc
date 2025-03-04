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
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, GlobalLetDecl) {
    auto p = parser("let a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: module-scope 'let' is invalid, use 'const'");
}

TEST_F(WGSLParserTest, GlobalConstDecl) {
    auto p = parser("const a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* c = e.value->As<ast::Const>();
    ASSERT_NE(c, nullptr);

    EXPECT_EQ(c->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(c->type, nullptr);
    ast::CheckIdentifier(c->type, "f32");

    EXPECT_EQ(c->source.range.begin.line, 1u);
    EXPECT_EQ(c->source.range.begin.column, 1u);
    EXPECT_EQ(c->source.range.end.line, 1u);
    EXPECT_EQ(c->source.range.end.column, 19u);

    EXPECT_EQ(c->name->source.range.begin.line, 1u);
    EXPECT_EQ(c->name->source.range.begin.column, 7u);
    EXPECT_EQ(c->name->source.range.end.line, 1u);
    EXPECT_EQ(c->name->source.range.end.column, 8u);

    ASSERT_NE(c->initializer, nullptr);
    EXPECT_TRUE(c->initializer->Is<ast::LiteralExpression>());
}

TEST_F(WGSLParserTest, GlobalConstDecl_Inferred) {
    auto p = parser("const a = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* c = e.value->As<ast::Const>();
    ASSERT_NE(c, nullptr);

    EXPECT_EQ(c->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(c->type, nullptr);

    EXPECT_EQ(c->source.range.begin.line, 1u);
    EXPECT_EQ(c->source.range.begin.column, 1u);
    EXPECT_EQ(c->source.range.end.line, 1u);
    EXPECT_EQ(c->source.range.end.column, 13u);

    EXPECT_EQ(c->name->source.range.begin.line, 1u);
    EXPECT_EQ(c->name->source.range.begin.column, 7u);
    EXPECT_EQ(c->name->source.range.end.line, 1u);
    EXPECT_EQ(c->name->source.range.end.column, 8u);

    ASSERT_NE(c->initializer, nullptr);
    EXPECT_TRUE(c->initializer->Is<ast::LiteralExpression>());
}

TEST_F(WGSLParserTest, GlobalConstDecl_InvalidExpression) {
    auto p = parser("const a : f32 = if (a) {}");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:17: missing initializer for 'const' declaration");
}

TEST_F(WGSLParserTest, GlobalConstDecl_MissingExpression) {
    auto p = parser("const a : f32 =");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_constant_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:16: missing initializer for 'const' declaration");
}

TEST_F(WGSLParserTest, GlobalOverrideDecl_WithId) {
    auto p = parser("@id(7) override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 8u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 29u);

    EXPECT_EQ(override->name->source.range.begin.line, 1u);
    EXPECT_EQ(override->name->source.range.begin.column, 17u);
    EXPECT_EQ(override->name->source.range.end.line, 1u);
    EXPECT_EQ(override->name->source.range.end.column, 18u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* override_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_NE(override_attr, nullptr);
    EXPECT_TRUE(override_attr->expr->Is<ast::IntLiteralExpression>());
}

TEST_F(WGSLParserTest, GlobalOverrideDecl_WithId_TrailingComma) {
    auto p = parser("@id(7,) override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 9u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 30u);

    EXPECT_EQ(override->name->source.range.begin.line, 1u);
    EXPECT_EQ(override->name->source.range.begin.column, 18u);
    EXPECT_EQ(override->name->source.range.end.line, 1u);
    EXPECT_EQ(override->name->source.range.end.column, 19u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* override_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_NE(override_attr, nullptr);
    EXPECT_TRUE(override_attr->expr->Is<ast::IntLiteralExpression>());
}

TEST_F(WGSLParserTest, GlobalOverrideDecl_WithoutId) {
    auto p = parser("override a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);

    auto e = p->global_constant_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* override = e.value->As<ast::Override>();
    ASSERT_NE(override, nullptr);

    EXPECT_EQ(override->name->symbol, p->builder().Symbols().Get("a"));
    ASSERT_NE(override->type, nullptr);
    ast::CheckIdentifier(override->type, "f32");

    EXPECT_EQ(override->source.range.begin.line, 1u);
    EXPECT_EQ(override->source.range.begin.column, 1u);
    EXPECT_EQ(override->source.range.end.line, 1u);
    EXPECT_EQ(override->source.range.end.column, 22u);

    EXPECT_EQ(override->name->source.range.begin.line, 1u);
    EXPECT_EQ(override->name->source.range.begin.column, 10u);
    EXPECT_EQ(override->name->source.range.end.line, 1u);
    EXPECT_EQ(override->name->source.range.end.column, 11u);

    ASSERT_NE(override->initializer, nullptr);
    EXPECT_TRUE(override->initializer->Is<ast::LiteralExpression>());

    auto* id_attr = ast::GetAttribute<ast::IdAttribute>(override->attributes);
    ASSERT_EQ(id_attr, nullptr);
}

}  // namespace
}  // namespace tint::wgsl::reader
