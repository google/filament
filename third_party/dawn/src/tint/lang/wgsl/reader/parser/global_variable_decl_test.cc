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

TEST_F(WGSLParserTest, GlobalVariableDecl_WithoutInitializer) {
    auto p = parser("var<private> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    ast::CheckIdentifier(var->name, "a");
    ast::CheckIdentifier(var->type, "f32");
    ast::CheckIdentifier(var->declared_address_space, "private");

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 1u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 21u);

    EXPECT_EQ(var->name->source.range.begin.line, 1u);
    EXPECT_EQ(var->name->source.range.begin.column, 14u);
    EXPECT_EQ(var->name->source.range.end.line, 1u);
    EXPECT_EQ(var->name->source.range.end.column, 15u);

    ASSERT_EQ(var->initializer, nullptr);
}

TEST_F(WGSLParserTest, GlobalVariableDecl_WithInitializer) {
    auto p = parser("var<private> a : f32 = 1.");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    ast::CheckIdentifier(var->name, "a");
    ast::CheckIdentifier(var->type, "f32");
    ast::CheckIdentifier(var->declared_address_space, "private");

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 1u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 26u);

    EXPECT_EQ(var->name->source.range.begin.line, 1u);
    EXPECT_EQ(var->name->source.range.begin.column, 14u);
    EXPECT_EQ(var->name->source.range.end.line, 1u);
    EXPECT_EQ(var->name->source.range.end.column, 15u);

    ASSERT_NE(var->initializer, nullptr);
    ASSERT_TRUE(var->initializer->Is<ast::FloatLiteralExpression>());
}

TEST_F(WGSLParserTest, GlobalVariableDecl_WithAttribute) {
    auto p = parser("@binding(2) @group(1) var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    ast::CheckIdentifier(var->name, "a");
    ast::CheckIdentifier(var->type, "f32");
    ast::CheckIdentifier(var->declared_address_space, "uniform");

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 23u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 43u);

    EXPECT_EQ(var->name->source.range.begin.line, 1u);
    EXPECT_EQ(var->name->source.range.begin.column, 36u);
    EXPECT_EQ(var->name->source.range.end.line, 1u);
    EXPECT_EQ(var->name->source.range.end.column, 37u);

    ASSERT_EQ(var->initializer, nullptr);

    auto& attributes = var->attributes;
    ASSERT_EQ(attributes.Length(), 2u);
    ASSERT_TRUE(attributes[0]->Is<ast::BindingAttribute>());
    ASSERT_TRUE(attributes[1]->Is<ast::GroupAttribute>());
}

TEST_F(WGSLParserTest, GlobalVariableDecl_WithAttribute_MulitpleGroups) {
    auto p = parser("@binding(2) @group(1) var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);

    auto e = p->global_variable_decl(attrs.value);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    auto* var = e.value->As<ast::Var>();
    ASSERT_NE(var, nullptr);

    ast::CheckIdentifier(var->name, "a");
    ast::CheckIdentifier(var->type, "f32");
    ast::CheckIdentifier(var->declared_address_space, "uniform");

    EXPECT_EQ(var->source.range.begin.line, 1u);
    EXPECT_EQ(var->source.range.begin.column, 23u);
    EXPECT_EQ(var->source.range.end.line, 1u);
    EXPECT_EQ(var->source.range.end.column, 43u);

    EXPECT_EQ(var->name->source.range.begin.line, 1u);
    EXPECT_EQ(var->name->source.range.begin.column, 36u);
    EXPECT_EQ(var->name->source.range.end.line, 1u);
    EXPECT_EQ(var->name->source.range.end.column, 37u);

    ASSERT_EQ(var->initializer, nullptr);

    auto& attributes = var->attributes;
    ASSERT_EQ(attributes.Length(), 2u);
    ASSERT_TRUE(attributes[0]->Is<ast::BindingAttribute>());
    ASSERT_TRUE(attributes[1]->Is<ast::GroupAttribute>());
}

TEST_F(WGSLParserTest, GlobalVariableDecl_InvalidAttribute) {
    auto p = parser("@binding() var<uniform> a : f32");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);

    auto e = p->global_variable_decl(attrs.value);
    EXPECT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    EXPECT_NE(e.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: binding expects 1 argument");
}

TEST_F(WGSLParserTest, GlobalVariableDecl_InvalidConstExpr) {
    auto p = parser("var<private> a : f32 = if (a) {}");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto e = p->global_variable_decl(attrs.value);
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:24: missing initializer for 'var' declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
