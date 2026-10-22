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

TEST_F(WGSLParserTest, ParamList_Single) {
    auto p = parser("a : i32");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.Length(), 1u);

    EXPECT_EQ(e.value[0]->name->symbol, p->builder().Symbols().Get("a"));
    ast::CheckIdentifier(e.value[0]->type, "i32");
    EXPECT_TRUE(e.value[0]->Is<ast::Parameter>());

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 2u);
}

TEST_F(WGSLParserTest, ParamList_Multiple) {
    auto p = parser("a : i32, b: f32, c: vec2<f32>");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.Length(), 3u);

    EXPECT_EQ(e.value[0]->name->symbol, p->builder().Symbols().Get("a"));
    ast::CheckIdentifier(e.value[0]->type, "i32");
    EXPECT_TRUE(e.value[0]->Is<ast::Parameter>());

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 2u);

    EXPECT_EQ(e.value[1]->name->symbol, p->builder().Symbols().Get("b"));
    ast::CheckIdentifier(e.value[1]->type, "f32");
    EXPECT_TRUE(e.value[1]->Is<ast::Parameter>());

    ASSERT_EQ(e.value[1]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[1]->source.range.begin.column, 10u);
    ASSERT_EQ(e.value[1]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[1]->source.range.end.column, 11u);

    EXPECT_EQ(e.value[2]->name->symbol, p->builder().Symbols().Get("c"));
    ast::CheckIdentifier(e.value[2]->type, ast::Template("vec2", "f32"));
    EXPECT_TRUE(e.value[2]->Is<ast::Parameter>());

    ASSERT_EQ(e.value[2]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[2]->source.range.begin.column, 18u);
    ASSERT_EQ(e.value[2]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[2]->source.range.end.column, 19u);
}

TEST_F(WGSLParserTest, ParamList_Empty) {
    auto p = parser("");
    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.Length(), 0u);
}

TEST_F(WGSLParserTest, ParamList_TrailingComma) {
    auto p = parser("a : i32,");
    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.Length(), 1u);
}

TEST_F(WGSLParserTest, ParamList_Attributes) {
    auto p = parser("@builtin(position) coord : vec4<f32>, @location(1) loc1 : f32");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_EQ(e.value.Length(), 2u);

    EXPECT_EQ(e.value[0]->name->symbol, p->builder().Symbols().Get("coord"));
    ast::CheckIdentifier(e.value[0]->type, ast::Template("vec4", "f32"));
    EXPECT_TRUE(e.value[0]->Is<ast::Parameter>());
    auto attrs_0 = e.value[0]->attributes;
    ASSERT_EQ(attrs_0.Length(), 1u);
    EXPECT_TRUE(attrs_0[0]->Is<ast::BuiltinAttribute>());
    EXPECT_EQ(attrs_0[0]->As<ast::BuiltinAttribute>()->builtin, core::BuiltinValue::kPosition);

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 20u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 25u);

    EXPECT_EQ(e.value[1]->name->symbol, p->builder().Symbols().Get("loc1"));
    ast::CheckIdentifier(e.value[1]->type, "f32");
    EXPECT_TRUE(e.value[1]->Is<ast::Parameter>());
    auto attrs_1 = e.value[1]->attributes;
    ASSERT_EQ(attrs_1.Length(), 1u);

    ASSERT_TRUE(attrs_1[0]->Is<ast::LocationAttribute>());
    auto* attr = attrs_1[0]->As<ast::LocationAttribute>();
    ASSERT_TRUE(attr->expr->Is<ast::IntLiteralExpression>());
    auto* loc = attr->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(loc->value, 1u);

    EXPECT_EQ(e.value[1]->source.range.begin.line, 1u);
    EXPECT_EQ(e.value[1]->source.range.begin.column, 52u);
    EXPECT_EQ(e.value[1]->source.range.end.line, 1u);
    EXPECT_EQ(e.value[1]->source.range.end.column, 56u);
}

}  // namespace
}  // namespace tint::wgsl::reader
