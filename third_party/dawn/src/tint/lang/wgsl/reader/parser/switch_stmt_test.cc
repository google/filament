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

TEST_F(WGSLParserTest, SwitchStmt_WithoutDefault) {
    auto p = parser(R"(switch a {
  case 1: {}
  case 2: {}
})");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 2u);
    EXPECT_FALSE(e->body[0]->ContainsDefault());
    EXPECT_FALSE(e->body[1]->ContainsDefault());
}

TEST_F(WGSLParserTest, SwitchStmt_Empty) {
    auto p = parser("switch a { }");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 0u);
}

TEST_F(WGSLParserTest, SwitchStmt_DefaultInMiddle) {
    auto p = parser(R"(switch a {
  case 1: {}
  default: {}
  case 2: {}
})");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    ASSERT_EQ(e->body.Length(), 3u);
    ASSERT_FALSE(e->body[0]->ContainsDefault());
    ASSERT_TRUE(e->body[1]->ContainsDefault());
    ASSERT_FALSE(e->body[2]->ContainsDefault());
}

TEST_F(WGSLParserTest, SwitchStmt_Default_Mixed) {
    auto p = parser(R"(switch a {
  case 1, default, 2: {}
})");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    ASSERT_EQ(e->body.Length(), 1u);
    ASSERT_TRUE(e->body[0]->ContainsDefault());
}

TEST_F(WGSLParserTest, SwitchStmt_WithParens) {
    auto p = parser("switch(a+b) { }");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 0u);
}

TEST_F(WGSLParserTest, SwitchStmt_WithAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) switch a { default{} }");
    auto a = p->attribute_list();
    auto e = p->switch_statement(a.value);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    EXPECT_TRUE(a->IsEmpty());
    ASSERT_EQ(e->attributes.Length(), 1u);
    EXPECT_TRUE(e->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, SwitchStmt_WithBodyAttributes) {
    auto p = parser("switch a @diagnostic(off, derivative_uniformity) { default{} }");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    EXPECT_TRUE(e->attributes.IsEmpty());
    ASSERT_EQ(e->body_attributes.Length(), 1u);
    EXPECT_TRUE(e->body_attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, SwitchStmt_InvalidExpression) {
    auto p = parser("switch a=b {}");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '{' for switch statement");
}

TEST_F(WGSLParserTest, SwitchStmt_MissingExpression) {
    auto p = parser("switch {}");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unable to parse selector expression");
}

TEST_F(WGSLParserTest, SwitchStmt_MissingBracketLeft) {
    auto p = parser("switch a }");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '{' for switch statement");
}

TEST_F(WGSLParserTest, SwitchStmt_MissingBracketRight) {
    auto p = parser("switch a {");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected '}' for switch statement");
}

TEST_F(WGSLParserTest, SwitchStmt_InvalidBody) {
    auto p = parser(R"(switch a {
  case: {}
})");
    Parser::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "2:7: expected case selector expression or `default`");
}

}  // namespace
}  // namespace tint::wgsl::reader
