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

TEST_F(WGSLParserTest, IfStmt) {
    auto p = parser("if a == 4 { a = b; c = d; }");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);
    EXPECT_EQ(e->else_statement, nullptr);
}

TEST_F(WGSLParserTest, IfStmt_WithElse) {
    auto p = parser("if a == 4 { a = b; c = d; } else if(c) { d = 2; } else {}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);

    auto* elseif = As<ast::IfStatement>(e->else_statement);
    ASSERT_NE(elseif, nullptr);
    ASSERT_TRUE(elseif->condition->Is<ast::IdentifierExpression>());
    EXPECT_EQ(elseif->body->statements.Length(), 1u);

    auto* el = As<ast::BlockStatement>(elseif->else_statement);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->statements.Length(), 0u);
}

TEST_F(WGSLParserTest, IfStmt_WithElse_WithParens) {
    auto p = parser("if(a==4) { a = b; c = d; } else if(c) { d = 2; } else {}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);

    auto* elseif = As<ast::IfStatement>(e->else_statement);
    ASSERT_NE(elseif, nullptr);
    ASSERT_TRUE(elseif->condition->Is<ast::IdentifierExpression>());
    EXPECT_EQ(elseif->body->statements.Length(), 1u);

    auto* el = As<ast::BlockStatement>(elseif->else_statement);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->statements.Length(), 0u);
}

TEST_F(WGSLParserTest, IfStmt_WithAttributes) {
    auto p = parser(R"(@diagnostic(off, derivative_uniformity) if true { })");
    auto a = p->attribute_list();
    auto e = p->if_statement(a.value);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IfStatement>());

    EXPECT_TRUE(a->IsEmpty());
    ASSERT_EQ(e->attributes.Length(), 1u);
    EXPECT_TRUE(e->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, IfStmt_InvalidCondition) {
    auto p = parser("if a = 3 {}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected '{' for if statement");
}

TEST_F(WGSLParserTest, IfStmt_MissingCondition) {
    auto p = parser("if {}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: unable to parse condition expression");
}

TEST_F(WGSLParserTest, IfStmt_InvalidBody) {
    auto p = parser("if a { fn main() {}}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected '}' for if statement");
}

TEST_F(WGSLParserTest, IfStmt_MissingBody) {
    auto p = parser("if a");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected '{' for if statement");
}

TEST_F(WGSLParserTest, IfStmt_InvalidElseif) {
    auto p = parser("if a {} else if a { fn main() -> a{}}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected '}' for if statement");
}

TEST_F(WGSLParserTest, IfStmt_InvalidElse) {
    auto p = parser("if a {} else { fn main() -> a{}}");
    Parser::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected '}' for else statement");
}

}  // namespace
}  // namespace tint::wgsl::reader
