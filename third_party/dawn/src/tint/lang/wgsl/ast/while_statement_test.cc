// Copyright 2022 The Dawn & Tint Authors
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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using WhileStatementTest = TestHelper;
using WhileStatementDeathTest = WhileStatementTest;

TEST_F(WhileStatementTest, Creation) {
    auto* cond = create<BinaryExpression>(core::BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(cond, body);

    EXPECT_EQ(l->condition, cond);
    EXPECT_EQ(l->body, body);
}

TEST_F(WhileStatementTest, Creation_WithSource) {
    auto* cond = create<BinaryExpression>(core::BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(Source{{20u, 2u}}, cond, body);
    auto src = l->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(WhileStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* cond = create<BinaryExpression>(core::BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* body = Block(Return());
    auto* l = While(cond, body, tint::Vector{attr1, attr2});

    EXPECT_THAT(l->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(WhileStatementDeathTest, Assert_Null_Cond) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            auto* body = b.Block();
            b.While(nullptr, body);
        },
        "internal compiler error");
}

TEST_F(WhileStatementDeathTest, Assert_Null_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            auto* cond =
                b.create<BinaryExpression>(core::BinaryOp::kLessThan, b.Expr("i"), b.Expr(5_u));
            b.While(cond, nullptr);
        },
        "internal compiler error");
}

TEST_F(WhileStatementDeathTest, Assert_DifferentGenerationID_Condition) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.While(b2.Expr(true), b1.Block());
        },
        "internal compiler error");
}

TEST_F(WhileStatementDeathTest, Assert_DifferentGenerationID_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.While(b1.Expr(true), b2.Block());
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
