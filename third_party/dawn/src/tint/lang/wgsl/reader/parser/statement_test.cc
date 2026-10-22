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

#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, Statement) {
    auto p = parser("return;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, Statement_Semicolon) {
    auto p = parser(";");
    p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
}

TEST_F(WGSLParserTest, Statement_Return_NoValue) {
    auto p = parser("return;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::ReturnStatement>());
    auto* ret = e->As<ast::ReturnStatement>();
    ASSERT_EQ(ret->value, nullptr);
}

TEST_F(WGSLParserTest, Statement_Return_Value) {
    auto p = parser("return a + b * (.1 - .2);");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::ReturnStatement>());
    auto* ret = e->As<ast::ReturnStatement>();
    ASSERT_NE(ret->value, nullptr);
    EXPECT_TRUE(ret->value->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, Statement_Return_MissingSemi) {
    auto p = parser("return");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:7: expected ';' for return statement");
}

TEST_F(WGSLParserTest, Statement_Return_Invalid) {
    auto p = parser("return if(a) {};");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:8: expected ';' for return statement");
}

TEST_F(WGSLParserTest, Statement_If) {
    auto p = parser("if (a) {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::IfStatement>());
}

TEST_F(WGSLParserTest, Statement_If_Invalid) {
    auto p = parser("if (a) { fn main() -> {}}");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:10: expected '}' for if statement");
}

TEST_F(WGSLParserTest, Statement_Variable) {
    auto p = parser("var a : i32 = 1;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
}

TEST_F(WGSLParserTest, Statement_Variable_Invalid) {
    auto p = parser("var a : i32 =;");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:14: missing initializer for 'var' declaration");
}

TEST_F(WGSLParserTest, Statement_Variable_MissingSemicolon) {
    auto p = parser("var a : i32");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:12: expected ';' for variable declaration");
}

TEST_F(WGSLParserTest, Statement_Switch) {
    auto p = parser("switch (a) {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
}

TEST_F(WGSLParserTest, Statement_Switch_Invalid) {
    auto p = parser("switch (a) { case: {}}");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:18: expected case selector expression or `default`");
}

TEST_F(WGSLParserTest, Statement_Loop) {
    auto p = parser("loop {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::LoopStatement>());
}

TEST_F(WGSLParserTest, Statement_Loop_Invalid) {
    auto p = parser("loop discard; }");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: expected '{' for loop");
}

TEST_F(WGSLParserTest, Statement_Assignment) {
    auto p = parser("a = b;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::AssignmentStatement>());
}

TEST_F(WGSLParserTest, Statement_Assignment_Invalid) {
    auto p = parser("a = if(b) {};");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:5: unable to parse right side of assignment");
}

TEST_F(WGSLParserTest, Statement_Assignment_MissingSemicolon) {
    auto p = parser("a = b");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: expected ';' for assignment statement");
}

TEST_F(WGSLParserTest, Statement_Break) {
    auto p = parser("break;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::BreakStatement>());
}

TEST_F(WGSLParserTest, Statement_Break_MissingSemicolon) {
    auto p = parser("break");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: expected ';' for break statement");
}

TEST_F(WGSLParserTest, Statement_Continue) {
    auto p = parser("continue;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::ContinueStatement>());
}

TEST_F(WGSLParserTest, Statement_Continue_MissingSemicolon) {
    auto p = parser("continue");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:9: expected ';' for continue statement");
}

TEST_F(WGSLParserTest, Statement_Discard) {
    auto p = parser("discard;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::DiscardStatement>());
}

TEST_F(WGSLParserTest, Statement_Discard_MissingSemicolon) {
    auto p = parser("discard");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(p->error(), "1:8: expected ';' for discard statement");
}

TEST_F(WGSLParserTest, Statement_Body) {
    auto p = parser("{ var i: i32; }");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::BlockStatement>());
    EXPECT_TRUE(e->As<ast::BlockStatement>()->statements[0]->Is<ast::VariableDeclStatement>());
}

TEST_F(WGSLParserTest, Statement_Body_Invalid) {
    auto p = parser("{ fn main() -> {}}");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '}' for block statement");
}

TEST_F(WGSLParserTest, Statement_ConstAssert_WithParen) {
    auto p = parser("const_assert(true);");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* sa = As<ast::ConstAssert>(e.value);
    ASSERT_NE(sa, nullptr);
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

TEST_F(WGSLParserTest, Statement_ConstAssert_WithoutParen) {
    auto p = parser("const_assert  true;");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* sa = As<ast::ConstAssert>(e.value);
    ASSERT_NE(sa, nullptr);
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

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_Block) {
    auto p = parser("@diagnostic(off, derivative_uniformity) {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::BlockStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_For) {
    auto p = parser("@diagnostic(off, derivative_uniformity) for (;false;) {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::ForLoopStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_If) {
    auto p = parser("@diagnostic(off, derivative_uniformity) if true {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::IfStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_Loop) {
    auto p = parser("@diagnostic(off, derivative_uniformity) loop {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::LoopStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_Switch) {
    auto p = parser("@diagnostic(off, derivative_uniformity) switch (0) { default{} }");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::SwitchStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_ConsumedAttributes_While) {
    auto p = parser("@diagnostic(off, derivative_uniformity) while (false) {}");
    auto e = p->statement();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);

    auto* s = As<ast::WhileStatement>(e.value);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->attributes.Length(), 1u);
}

TEST_F(WGSLParserTest, Statement_UnexpectedAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) return;");
    auto e = p->statement();
    EXPECT_TRUE(p->has_error());
    EXPECT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    EXPECT_NE(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:2: unexpected attributes");
}

}  // namespace
}  // namespace tint::wgsl::reader
