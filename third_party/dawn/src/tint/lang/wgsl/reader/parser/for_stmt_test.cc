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

#include "src/tint/lang/wgsl/ast/discard_statement.h"

namespace tint::wgsl::reader {
namespace {

using ForStmtTest = WGSLParserTest;

// Test an empty for loop.
TEST_F(ForStmtTest, Empty) {
    auto p = parser("for (;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop with non-empty body.
TEST_F(ForStmtTest, Body) {
    auto p = parser("for (;;) { discard; }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    ASSERT_EQ(fl->body->statements.Length(), 1u);
    EXPECT_TRUE(fl->body->statements[0]->Is<ast::DiscardStatement>());
}

// Test a for loop declaring a variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementDecl) {
    auto p = parser("for (var i: i32 ;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    ASSERT_TRUE(Is<ast::VariableDeclStatement>(fl->initializer));
    auto* var = fl->initializer->As<ast::VariableDeclStatement>()->variable;
    EXPECT_TRUE(var->Is<ast::Var>());
    EXPECT_EQ(var->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop declaring and initializing a variable in the initializer
// statement.
TEST_F(ForStmtTest, InitializerStatementDeclEqual) {
    auto p = parser("for (var i: i32 = 0 ;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    ASSERT_TRUE(Is<ast::VariableDeclStatement>(fl->initializer));
    auto* var = fl->initializer->As<ast::VariableDeclStatement>()->variable;
    EXPECT_TRUE(var->Is<ast::Var>());
    EXPECT_NE(var->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop declaring a const variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementConstDecl) {
    auto p = parser("for (let i: i32 = 0 ;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    ASSERT_TRUE(Is<ast::VariableDeclStatement>(fl->initializer));
    auto* var = fl->initializer->As<ast::VariableDeclStatement>()->variable;
    EXPECT_TRUE(var->Is<ast::Let>());
    EXPECT_NE(var->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop assigning a variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementAssignment) {
    auto p = parser("for (i = 0 ;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_TRUE(Is<ast::AssignmentStatement>(fl->initializer));
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop incrementing a variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementIncrement) {
    auto p = parser("for (i++;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_TRUE(Is<ast::IncrementDecrementStatement>(fl->initializer));
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop calling a function in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementFuncCall) {
    auto p = parser("for (a(b,c) ;;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_TRUE(Is<ast::CallStatement>(fl->initializer));
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop with a break condition
TEST_F(ForStmtTest, BreakCondition) {
    auto p = parser("for (; 0 == 1;) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_TRUE(Is<ast::BinaryExpression>(fl->condition));
    EXPECT_EQ(fl->continuing, nullptr);
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop assigning a variable in the continuing statement.
TEST_F(ForStmtTest, ContinuingAssignment) {
    auto p = parser("for (;; x = 2) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_TRUE(Is<ast::AssignmentStatement>(fl->continuing));
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop with an increment statement as the continuing statement.
TEST_F(ForStmtTest, ContinuingIncrement) {
    auto p = parser("for (;; x++) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_TRUE(Is<ast::IncrementDecrementStatement>(fl->continuing));
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop calling a function in the continuing statement.
TEST_F(ForStmtTest, ContinuingFuncCall) {
    auto p = parser("for (;; a(b,c)) { }");
    Parser::AttributeList attrs;
    auto fl = p->for_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);
    EXPECT_EQ(fl->initializer, nullptr);
    EXPECT_EQ(fl->condition, nullptr);
    EXPECT_TRUE(Is<ast::CallStatement>(fl->continuing));
    EXPECT_TRUE(fl->body->Empty());
}

// Test a for loop with attributes.
TEST_F(ForStmtTest, WithAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) for (;;) { }");
    auto attrs = p->attribute_list();
    auto fl = p->for_statement(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(fl.errored);
    ASSERT_TRUE(fl.matched);

    EXPECT_TRUE(attrs->IsEmpty());
    ASSERT_EQ(fl->attributes.Length(), 1u);
    EXPECT_TRUE(fl->attributes[0]->Is<ast::DiagnosticAttribute>());
}

class ForStmtErrorTest : public WGSLParserTest {
  public:
    void TestForWithError(std::string for_str, std::string error_str) {
        auto p_for = parser(for_str);
        Parser::AttributeList attrs;
        auto e_for = p_for->for_statement(attrs);

        EXPECT_FALSE(e_for.matched);
        EXPECT_TRUE(e_for.errored);
        EXPECT_TRUE(p_for->has_error());
        ASSERT_EQ(e_for.value, nullptr);
        EXPECT_EQ(p_for->error(), error_str);
    }
};

// Test a for loop with missing left parenthesis is invalid.
TEST_F(ForStmtErrorTest, MissingLeftParen) {
    std::string for_str = "for { }";
    std::string error_str = "1:5: expected '(' for for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with missing first semicolon is invalid.
TEST_F(ForStmtErrorTest, MissingFirstSemicolon) {
    std::string for_str = "for () {}";
    std::string error_str = "1:6: expected ';' for initializer in for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with missing second semicolon is invalid.
TEST_F(ForStmtErrorTest, MissingSecondSemicolon) {
    std::string for_str = "for (;) {}";
    std::string error_str = "1:7: expected ';' for condition in for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with missing right parenthesis is invalid.
TEST_F(ForStmtErrorTest, MissingRightParen) {
    std::string for_str = "for (;; {}";
    std::string error_str = "1:9: expected ')' for for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with missing left brace is invalid.
TEST_F(ForStmtErrorTest, MissingLeftBrace) {
    std::string for_str = "for (;;)";
    std::string error_str = "1:9: expected '{' for for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with missing right brace is invalid.
TEST_F(ForStmtErrorTest, MissingRightBrace) {
    std::string for_str = "for (;;) {";
    std::string error_str = "1:11: expected '}' for for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid initializer statement.
TEST_F(ForStmtErrorTest, InvalidInitializerAsConstDecl) {
    std::string for_str = "for (let x: i32;;) { }";
    std::string error_str = "1:16: expected '=' for 'let' declaration";

    TestForWithError(for_str, error_str);
}

// Test a for loop with a initializer statement not matching
// variable_stmt | assignment_stmt | func_call_stmt.
TEST_F(ForStmtErrorTest, InvalidInitializerMatch) {
    std::string for_str = "for (if (true) {} ;;) { }";
    std::string error_str = "1:6: expected ';' for initializer in for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid break condition.
TEST_F(ForStmtErrorTest, InvalidBreakConditionAsExpression) {
    std::string for_str = "for (; (0 == 1; ) { }";
    std::string error_str = "1:15: expected ')'";

    TestForWithError(for_str, error_str);
}

// Test a for loop with a break condition not matching
// logical_or_expression.
TEST_F(ForStmtErrorTest, InvalidBreakConditionMatch) {
    std::string for_str = "for (; var i: i32 = 0;) { }";
    std::string error_str = "1:8: expected ';' for condition in for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid continuing statement.
TEST_F(ForStmtErrorTest, InvalidContinuingAsFuncCall) {
    std::string for_str = "for (;; a(,) ) { }";
    std::string error_str = "1:11: expected expression for function call";

    TestForWithError(for_str, error_str);
}

// Test a for loop with a continuing statement not matching
// assignment_stmt | func_call_stmt.
TEST_F(ForStmtErrorTest, InvalidContinuingMatch) {
    std::string for_str = "for (;; var i: i32 = 0) { }";
    std::string error_str = "1:9: expected ')' for for loop";

    TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid body.
TEST_F(ForStmtErrorTest, InvalidBody) {
    std::string for_str = "for (;;) { let x: i32; }";
    std::string error_str = "1:22: expected '=' for 'let' declaration";

    TestForWithError(for_str, error_str);
}

// Test a for loop with a body not matching statements
TEST_F(ForStmtErrorTest, InvalidBodyMatch) {
    std::string for_str = "for (;;) { fn main() {} }";
    std::string error_str = "1:12: expected '}' for for loop";

    TestForWithError(for_str, error_str);
}

}  // namespace
}  // namespace tint::wgsl::reader
