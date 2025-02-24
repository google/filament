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

#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, Statement_Call) {
    auto p = parser("a();");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 2u);

    ASSERT_TRUE(e->Is<ast::CallStatement>());
    auto* c = e->As<ast::CallStatement>()->expr;

    ast::CheckIdentifier(c->target, "a");

    EXPECT_EQ(c->args.Length(), 0u);
}

TEST_F(WGSLParserTest, Statement_Call_WithParams) {
    auto p = parser("a(1, b, 2 + 3 / b);");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    ASSERT_TRUE(e->Is<ast::CallStatement>());
    auto* c = e->As<ast::CallStatement>()->expr;

    ast::CheckIdentifier(c->target, "a");

    EXPECT_EQ(c->args.Length(), 3u);
    EXPECT_TRUE(c->args[0]->Is<ast::IntLiteralExpression>());
    EXPECT_TRUE(c->args[1]->Is<ast::IdentifierExpression>());
    EXPECT_TRUE(c->args[2]->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, Statement_Call_WithParams_TrailingComma) {
    auto p = parser("a(1, b,);");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    ASSERT_TRUE(e->Is<ast::CallStatement>());
    auto* c = e->As<ast::CallStatement>()->expr;

    ast::CheckIdentifier(c->target, "a");

    EXPECT_EQ(c->args.Length(), 2u);
    EXPECT_TRUE(c->args[0]->Is<ast::IntLiteralExpression>());
    EXPECT_TRUE(c->args[1]->Is<ast::IdentifierExpression>());
}

TEST_F(WGSLParserTest, Statement_Call_Missing_RightParen) {
    auto p = parser("a(");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(p->error(), "1:3: expected expression for function call");
}

TEST_F(WGSLParserTest, Statement_Call_Missing_Semi) {
    auto p = parser("a()");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(p->error(), "1:4: expected ';' for function call");
}

TEST_F(WGSLParserTest, Statement_Call_Bad_ArgList) {
    auto p = parser("a(b c);");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(p->error(), "1:5: expected ',' for function call");
}

}  // namespace
}  // namespace tint::wgsl::reader
