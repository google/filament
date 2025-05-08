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

TEST_F(WGSLParserTest, AssignmentStmt_Parses_ToVariable) {
    auto p = parser("a = 123");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, AssignmentStmt_Parses_ToMember) {
    auto p = parser("a.b.c[2].d = 123");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 12u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 13u);

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(a->lhs->Is<ast::MemberAccessorExpression>());
    auto* mem = a->lhs->As<ast::MemberAccessorExpression>();

    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("d"));

    ASSERT_TRUE(mem->object->Is<ast::IndexAccessorExpression>());
    auto* idx = mem->object->As<ast::IndexAccessorExpression>();

    ASSERT_NE(idx->index, nullptr);
    ASSERT_TRUE(idx->index->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(idx->object->Is<ast::MemberAccessorExpression>());
    mem = idx->object->As<ast::MemberAccessorExpression>();
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("c"));

    ASSERT_TRUE(mem->object->Is<ast::MemberAccessorExpression>());

    mem = mem->object->As<ast::MemberAccessorExpression>();
    ASSERT_TRUE(mem->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = mem->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(WGSLParserTest, AssignmentStmt_SuffixIncrement) {
    auto p = parser("a++");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_TRUE(a->increment);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 2u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

TEST_F(WGSLParserTest, AssignmentStmt_SuffixDecrement) {
    auto p = parser("a--");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::IncrementDecrementStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_FALSE(a->increment);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 2u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

TEST_F(WGSLParserTest, AssignmentStmt_PerfixIncrementFails) {
    auto p = parser("++a");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: prefix increment and decrement operators are not supported");
}

TEST_F(WGSLParserTest, AssignmentStmt_PerfixDecrementFails) {
    auto p = parser("--a");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: prefix increment and decrement operators are not supported");
}

TEST_F(WGSLParserTest, AssignmentStmt_Parses_ToPhony) {
    auto p = parser("_ = 123i");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kI);

    ASSERT_TRUE(a->lhs->Is<ast::PhonyExpression>());
}

TEST_F(WGSLParserTest, AssignmentStmt_Phony_CompoundOpFails) {
    auto p = parser("_ += 123i");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(WGSLParserTest, AssignmentStmt_Phony_IncrementFails) {
    auto p = parser("_ ++");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(WGSLParserTest, AssignmentStmt_Phony_EqualPrefixIncrementFails) {
    auto p = parser("_ = ++a");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:5: prefix increment and decrement operators are not supported");
}

TEST_F(WGSLParserTest, AssignmentStmt_Phony_EqualPrefixDecrement) {
    // This is valid since (--a) is parsed as (-(-(a))).
    auto p = parser("_ = --a");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::PhonyExpression>());

    // The RHS of phony assignment should be a nested Negation unary expression.
    ASSERT_TRUE(a->rhs->Is<ast::UnaryOpExpression>());
    // The outer should be a nNegation of inner unary expression.
    auto* outer = a->rhs->As<ast::UnaryOpExpression>();
    ASSERT_EQ(outer->op, core::UnaryOp::kNegation);
    ASSERT_TRUE(outer->expr->Is<ast::UnaryOpExpression>());
    // The inner unary expression should be Negation of identifier `a`.
    auto* inner = outer->expr->As<ast::UnaryOpExpression>();
    ASSERT_EQ(inner->op, core::UnaryOp::kNegation);
    ASSERT_TRUE(inner->expr->Is<ast::IdentifierExpression>());
    auto* ident_expr = inner->expr->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

// When parsing as variable updating statement, `_ = a++` will only be parsed as `_ = a`, and the
// last kPlusPlus token will be stay unused, as it will never be a part of a valid variable updating
// statement. However, in this case no error would be raised by variable_updating_statement().
TEST_F(WGSLParserTest, AssignmentStmt_Phony_EqualSuffixIncrementNotUsed) {
    auto p = parser("_ = a++");
    auto e = p->variable_updating_statement();
    // Parser will left the last kPlusPlus token unused.
    EXPECT_EQ(p->last_source().range.begin.line, 1u);
    EXPECT_EQ(p->last_source().range.begin.column, 5u);
    EXPECT_EQ(p->last_source().range.end.line, 1u);
    EXPECT_EQ(p->last_source().range.end.column, 6u);

    // The parsed part `_ = a` is valid.
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::PhonyExpression>());

    ASSERT_TRUE(a->rhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

TEST_F(WGSLParserTest, AssignmentStmt_Phony_EqualSuffixDecrementFails) {
    auto p = parser("_ = a--");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:8: unable to parse right side of - expression");
}

struct CompoundData {
    std::string str;
    core::BinaryOp op;
};
using CompoundOpTest = WGSLParserTestWithParam<CompoundData>;
TEST_P(CompoundOpTest, CompoundOp) {
    auto params = GetParam();
    auto p = parser("a " + params.str + " 123u");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::CompoundAssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);
    EXPECT_EQ(a->op, params.op);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 3u + params.str.length());

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kU);
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         CompoundOpTest,
                         testing::Values(CompoundData{"+=", core::BinaryOp::kAdd},
                                         CompoundData{"-=", core::BinaryOp::kSubtract},
                                         CompoundData{"*=", core::BinaryOp::kMultiply},
                                         CompoundData{"/=", core::BinaryOp::kDivide},
                                         CompoundData{"%=", core::BinaryOp::kModulo},
                                         CompoundData{"&=", core::BinaryOp::kAnd},
                                         CompoundData{"|=", core::BinaryOp::kOr},
                                         CompoundData{"^=", core::BinaryOp::kXor},
                                         CompoundData{">>=", core::BinaryOp::kShiftRight},
                                         CompoundData{"<<=", core::BinaryOp::kShiftLeft}));

TEST_F(WGSLParserTest, AssignmentStmt_MissingEqual) {
    auto p = parser("a.b.c[2].d 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:12: expected '=' for assignment");
}

TEST_F(WGSLParserTest, AssignmentStmt_Compound_MissingEqual) {
    auto p = parser("a + 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(WGSLParserTest, AssignmentStmt_InvalidLHS) {
    auto p = parser("if (true) {} = 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e.value, nullptr);
}

TEST_F(WGSLParserTest, AssignmentStmt_InvalidRHS) {
    auto p = parser("a.b.c[2].d = if (true) {}");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to parse right side of assignment");
}

TEST_F(WGSLParserTest, AssignmentStmt_InvalidCompoundOp) {
    auto p = parser("a &&= true");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

}  // namespace
}  // namespace tint::wgsl::reader
