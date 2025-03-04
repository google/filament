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

TEST_F(WGSLParserTest, VariableStmt_VariableDecl) {
    auto p = parser("var a : i32;");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);

    EXPECT_EQ(e->variable->initializer, nullptr);
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_WithInit) {
    auto p = parser("var a : i32 = 1;");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);

    ASSERT_NE(e->variable->initializer, nullptr);
    EXPECT_TRUE(e->variable->initializer->Is<ast::LiteralExpression>());
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_InitializerInvalid) {
    auto p = parser("var a : i32 = if(a) {}");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:15: missing initializer for 'var' declaration");
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_ArrayInit) {
    auto p = parser("var a : array<i32> = array<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("array", "i32"));
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_ArrayInit_NoSpace) {
    auto p = parser("var a : array<i32>=array<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("array", "i32"));
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_VecInit) {
    auto p = parser("var a : vec2<i32> = vec2<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ast::CheckIdentifier(call->target, ast::Template("vec2", "i32"));
}

TEST_F(WGSLParserTest, VariableStmt_VariableDecl_VecInit_NoSpace) {
    auto p = parser("var a : vec2<i32>=vec2<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("vec2", "i32"));
}

TEST_F(WGSLParserTest, VariableStmt_Let) {
    auto p = parser("let a : i32 = 1");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);
}

TEST_F(WGSLParserTest, VariableStmt_Let_ComplexExpression) {
    auto p = parser("let x = collide + collide_1;");
    // Parse as `statement` to validate the `;` at the end so we know we parsed the whole expression
    auto e = p->statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());

    auto* decl = e->As<ast::VariableDeclStatement>();
    ASSERT_NE(decl->variable->initializer, nullptr);

    ASSERT_TRUE(decl->variable->initializer->Is<ast::BinaryExpression>());
    auto* expr = decl->variable->initializer->As<ast::BinaryExpression>();
    EXPECT_EQ(expr->op, core::BinaryOp::kAdd);

    ASSERT_TRUE(expr->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = expr->lhs->As<ast::IdentifierExpression>();
    ast::CheckIdentifier(ident_expr->identifier, "collide");

    ASSERT_TRUE(expr->rhs->Is<ast::IdentifierExpression>());
    ident_expr = expr->rhs->As<ast::IdentifierExpression>();
    ast::CheckIdentifier(ident_expr->identifier, "collide_1");
}

TEST_F(WGSLParserTest, VariableStmt_Let_MissingEqual) {
    auto p = parser("let a : i32 1");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '=' for 'let' declaration");
}

TEST_F(WGSLParserTest, VariableStmt_Let_MissingInitializer) {
    auto p = parser("let a : i32 =");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: missing initializer for 'let' declaration");
}

TEST_F(WGSLParserTest, VariableStmt_Let_InvalidInitializer) {
    auto p = parser("let a : i32 = if (a) {}");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:15: missing initializer for 'let' declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
