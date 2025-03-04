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

namespace tint::ast {
namespace {

using BinaryExpressionTest = TestHelper;
using BinaryExpressionDeathTest = BinaryExpressionTest;

TEST_F(BinaryExpressionTest, Creation) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* r = create<BinaryExpression>(core::BinaryOp::kEqual, lhs, rhs);
    EXPECT_EQ(r->lhs, lhs);
    EXPECT_EQ(r->rhs, rhs);
    EXPECT_EQ(r->op, core::BinaryOp::kEqual);
}

TEST_F(BinaryExpressionTest, Creation_WithSource) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* r =
        create<BinaryExpression>(Source{Source::Location{20, 2}}, core::BinaryOp::kEqual, lhs, rhs);
    auto src = r->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BinaryExpressionTest, IsBinary) {
    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    auto* r = create<BinaryExpression>(core::BinaryOp::kEqual, lhs, rhs);
    EXPECT_TRUE(r->Is<BinaryExpression>());
}

TEST_F(BinaryExpressionDeathTest, Assert_Null_LHS) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<BinaryExpression>(core::BinaryOp::kEqual, nullptr, b.Expr("rhs"));
        },
        "internal compiler error");
}

TEST_F(BinaryExpressionDeathTest, Assert_Null_RHS) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<BinaryExpression>(core::BinaryOp::kEqual, b.Expr("lhs"), nullptr);
        },
        "internal compiler error");
}

TEST_F(BinaryExpressionDeathTest, Assert_DifferentGenerationID_LHS) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<BinaryExpression>(core::BinaryOp::kEqual, b2.Expr("lhs"), b1.Expr("rhs"));
        },
        "internal compiler error");
}

TEST_F(BinaryExpressionDeathTest, Assert_DifferentGenerationID_RHS) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<BinaryExpression>(core::BinaryOp::kEqual, b1.Expr("lhs"), b2.Expr("rhs"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
