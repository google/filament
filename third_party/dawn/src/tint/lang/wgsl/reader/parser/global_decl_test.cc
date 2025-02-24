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

TEST_F(WGSLParserTest, GlobalDecl_Semicolon) {
    auto p = parser(";");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalVariable) {
    auto p = parser("var<private> a : vec2<i32> = vec2<i32>(1, 2);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
    ast::CheckIdentifier(v->type, ast::Template("vec2", "i32"));
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalVariable_Inferred) {
    auto p = parser("var<private> a = vec2<i32>(1, 2);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
    EXPECT_EQ(v->type, nullptr);
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalVariable_MissingSemicolon) {
    auto p = parser("var<private> a : vec2<i32>");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:27: expected ';' for variable declaration");
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalLet) {
    auto p = parser("let a : i32 = 2;");
    auto e = p->global_decl();
    EXPECT_TRUE(p->has_error());
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: module-scope 'let' is invalid, use 'const'");
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalConst) {
    auto p = parser("const a : i32 = 2;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalConst_MissingInitializer) {
    auto p = parser("const a : vec2<i32>;");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:20: expected '=' for 'const' declaration");
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalConst_Invalid) {
    auto p = parser("const a : vec2<i32> 1.0;");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected '=' for 'const' declaration");
}

TEST_F(WGSLParserTest, GlobalDecl_GlobalConst_MissingSemicolon) {
    auto p = parser("const a : vec2<i32> = vec2<i32>(1, 2)");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:38: expected ';' for 'const' declaration");
}

TEST_F(WGSLParserTest, GlobalDecl_TypeAlias) {
    auto p = parser("alias A = i32;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 1u);
    ASSERT_TRUE(program.AST().TypeDecls()[0]->Is<ast::Alias>());
    ast::CheckIdentifier(program.AST().TypeDecls()[0]->As<ast::Alias>()->name, "A");
}

TEST_F(WGSLParserTest, GlobalDecl_TypeAlias_StructIdent) {
    auto p = parser(R"(struct A {
  a : f32,
}
alias B = A;)");
    p->global_decl();
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 2u);
    ASSERT_TRUE(program.AST().TypeDecls()[0]->Is<ast::Struct>());
    auto* str = program.AST().TypeDecls()[0]->As<ast::Struct>();
    EXPECT_EQ(str->name->symbol, program.Symbols().Get("A"));

    ASSERT_TRUE(program.AST().TypeDecls()[1]->Is<ast::Alias>());
    auto* alias = program.AST().TypeDecls()[1]->As<ast::Alias>();
    EXPECT_EQ(alias->name->symbol, program.Symbols().Get("B"));
    ast::CheckIdentifier(alias->type, "A");
}

TEST_F(WGSLParserTest, GlobalDecl_TypeAlias_MissingSemicolon) {
    auto p = parser("alias A = i32");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: expected ';' for type alias");
}

TEST_F(WGSLParserTest, GlobalDecl_Function) {
    auto p = parser("fn main() { return; }");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().Functions().Length(), 1u);
    ast::CheckIdentifier(program.AST().Functions()[0]->name, "main");
}

TEST_F(WGSLParserTest, GlobalDecl_Function_WithAttribute) {
    auto p = parser("@workgroup_size(2) fn main() { return; }");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().Functions().Length(), 1u);
    ast::CheckIdentifier(program.AST().Functions()[0]->name, "main");
}

TEST_F(WGSLParserTest, GlobalDecl_Function_Invalid) {
    auto p = parser("fn main() -> { return; }");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(WGSLParserTest, GlobalDecl_ParsesStruct) {
    auto p = parser("struct A { b: i32, c: f32}");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 1u);

    auto* t = program.AST().TypeDecls()[0];
    ASSERT_NE(t, nullptr);
    ASSERT_TRUE(t->Is<ast::Struct>());

    auto* str = t->As<ast::Struct>();
    EXPECT_EQ(str->name->symbol, program.Symbols().Get("A"));
    EXPECT_EQ(str->members.Length(), 2u);
}

TEST_F(WGSLParserTest, GlobalDecl_Struct_Invalid) {
    {
        auto p = parser("A {}");
        auto decl = p->global_decl();
        // global_decl will result in a no match.
        ASSERT_FALSE(p->has_error()) << p->error();
        ASSERT_TRUE(!decl.matched && !decl.errored);
    }
    {
        auto p = parser("A {}");
        p->translation_unit();
        // translation_unit will result in a general error.
        ASSERT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:1: unexpected token");
    }
}

TEST_F(WGSLParserTest, GlobalDecl_Struct_UnexpectedAttribute) {
    auto p = parser("@vertex struct S { i : i32 }");

    auto s = p->global_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unexpected attributes");
}

TEST_F(WGSLParserTest, GlobalDecl_ConstAssert_WithParen) {
    auto p = parser("const_assert(true);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().ConstAsserts().Length(), 1u);
    auto* sa = program.AST().ConstAsserts()[0];
    EXPECT_EQ(sa->source.range.begin.line, 1u);
    EXPECT_EQ(sa->source.range.begin.column, 1u);
    EXPECT_EQ(sa->source.range.end.line, 1u);
    EXPECT_EQ(sa->source.range.end.column, 19u);

    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());
    EXPECT_EQ(sa->condition->source.range.begin.line, 1u);
    EXPECT_EQ(sa->condition->source.range.begin.column, 14u);
    EXPECT_EQ(sa->condition->source.range.end.line, 1u);
    EXPECT_EQ(sa->condition->source.range.end.column, 18u);
}

TEST_F(WGSLParserTest, GlobalDecl_ConstAssert_WithoutParen) {
    auto p = parser("const_assert  true;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().ConstAsserts().Length(), 1u);
    auto* sa = program.AST().ConstAsserts()[0];
    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());

    EXPECT_EQ(sa->source.range.begin.line, 1u);
    EXPECT_EQ(sa->source.range.begin.column, 1u);
    EXPECT_EQ(sa->source.range.end.line, 1u);
    EXPECT_EQ(sa->source.range.end.column, 19u);

    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());
    EXPECT_EQ(sa->condition->source.range.begin.line, 1u);
    EXPECT_EQ(sa->condition->source.range.begin.column, 15u);
    EXPECT_EQ(sa->condition->source.range.end.line, 1u);
    EXPECT_EQ(sa->condition->source.range.end.column, 19u);
}

}  // namespace
}  // namespace tint::wgsl::reader
