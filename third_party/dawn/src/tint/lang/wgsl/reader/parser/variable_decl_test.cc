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
TEST_F(WGSLParserTest, VariableDecl_Parses) {
    auto p = parser("var my_var : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, "my_var");
    EXPECT_NE(v->type, nullptr);

    ast::CheckIdentifier(v->type, "f32");

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 11u}}));
    EXPECT_EQ(v->type->source.range, (Source::Range{{1u, 14u}, {1u, 17u}}));
}

TEST_F(WGSLParserTest, VariableDecl_Unicode_Parses) {
    const std::string ident =  // "𝖎𝖉𝖊𝖓𝖙𝖎𝖋𝖎𝖊𝖗123"
        "\xf0\x9d\x96\x8e\xf0\x9d\x96\x89\xf0\x9d\x96\x8a\xf0\x9d\x96\x93"
        "\xf0\x9d\x96\x99\xf0\x9d\x96\x8e\xf0\x9d\x96\x8b\xf0\x9d\x96\x8e"
        "\xf0\x9d\x96\x8a\xf0\x9d\x96\x97\x31\x32\x33";

    auto p = parser("var " + ident + " : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, ident);
    EXPECT_NE(v->type, nullptr);

    ast::CheckIdentifier(v->type, "f32");

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 48u}}));
    EXPECT_EQ(v->type->source.range, (Source::Range{{1u, 51u}, {1u, 54u}}));
}

TEST_F(WGSLParserTest, VariableDecl_Inferred_Parses) {
    auto p = parser("var my_var = 1.0");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, "my_var");
    EXPECT_EQ(v->type, nullptr);

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 11u}}));
}

TEST_F(WGSLParserTest, VariableDecl_MissingVar) {
    auto p = parser("my_var : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());

    auto& t = p->next();
    ASSERT_TRUE(t.IsIdentifier());
}

TEST_F(WGSLParserTest, VariableDecl_WithAddressSpace) {
    auto p = parser("var<private> my_var : f32");
    auto v = p->variable_decl();
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());
    ast::CheckIdentifier(v->name, "my_var");

    ast::CheckIdentifier(v->type, "f32");
    ast::CheckIdentifier(v->address_space, "private");

    EXPECT_EQ(v->source.range.begin.line, 1u);
    EXPECT_EQ(v->source.range.begin.column, 14u);
    EXPECT_EQ(v->source.range.end.line, 1u);
    EXPECT_EQ(v->source.range.end.column, 20u);
}

TEST_F(WGSLParserTest, VariableDecl_WithPushConstant) {
    auto p = parser("var<push_constant> my_var : f32");
    auto v = p->variable_decl();
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());
    ast::CheckIdentifier(v->name, "my_var");

    ast::CheckIdentifier(v->type, "f32");
    ast::CheckIdentifier(v->address_space, "push_constant");
}

TEST_F(WGSLParserTest, VariableDecl_WithAddressSpaceTrailingComma) {
    auto p = parser("var<private,> my_var : f32");
    auto v = p->variable_decl();
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());
    ast::CheckIdentifier(v->name, "my_var");

    ast::CheckIdentifier(v->type, "f32");
    ast::CheckIdentifier(v->address_space, "private");

    EXPECT_EQ(v->source.range.begin.line, 1u);
    EXPECT_EQ(v->source.range.begin.column, 15u);
    EXPECT_EQ(v->source.range.end.line, 1u);
    EXPECT_EQ(v->source.range.end.column, 21u);
}

}  // namespace
}  // namespace tint::wgsl::reader
