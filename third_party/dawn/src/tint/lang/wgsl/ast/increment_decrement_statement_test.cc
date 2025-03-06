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

#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using IncrementDecrementStatementTest = TestHelper;
using IncrementDecrementStatementDeathTest = IncrementDecrementStatementTest;

TEST_F(IncrementDecrementStatementTest, Creation) {
    auto* expr = Expr("expr");

    auto* i = create<IncrementDecrementStatement>(expr, true);
    EXPECT_EQ(i->lhs, expr);
    EXPECT_TRUE(i->increment);
}

TEST_F(IncrementDecrementStatementTest, Creation_WithSource) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(Source{Source::Location{20, 2}}, expr, true);
    auto src = i->source;
    EXPECT_EQ(i->lhs, expr);
    EXPECT_TRUE(i->increment);
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IncrementDecrementStatementTest, IsIncrementDecrement) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(expr, true);
    EXPECT_TRUE(i->Is<IncrementDecrementStatement>());
}

TEST_F(IncrementDecrementStatementTest, Decrement) {
    auto* expr = Expr("expr");
    auto* i = create<IncrementDecrementStatement>(expr, false);
    EXPECT_EQ(i->lhs, expr);
    EXPECT_FALSE(i->increment);
}

TEST_F(IncrementDecrementStatementDeathTest, Assert_DifferentGenerationID_Expr) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<IncrementDecrementStatement>(b2.Expr(true), true);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
