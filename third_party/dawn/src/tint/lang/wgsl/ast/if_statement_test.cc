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

#include "src/tint/lang/wgsl/ast/if_statement.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using IfStatementTest = TestHelper;
using IfStatementDeathTest = IfStatementTest;

TEST_F(IfStatementTest, Creation) {
    auto* cond = Expr("cond");
    auto* stmt = If(Source{Source::Location{20, 2}}, cond, Block(create<DiscardStatement>()));
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IfStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* cond = Expr("cond");
    auto* stmt = If(cond, Block(), ElseStmt(), tint::Vector{attr1, attr2});

    EXPECT_THAT(stmt->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(IfStatementTest, IsIf) {
    auto* stmt = If(Expr(true), Block());
    EXPECT_TRUE(stmt->Is<IfStatement>());
}

TEST_F(IfStatementDeathTest, Assert_Null_Condition) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.If(nullptr, b.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementDeathTest, Assert_Null_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.If(b.Expr(true), nullptr);
        },
        "internal compiler error");
}

TEST_F(IfStatementDeathTest, Assert_InvalidElse) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.If(b.Expr(true), b.Block(), b.Else(b.CallStmt(b.Call("foo"))));
        },
        "internal compiler error");
}

TEST_F(IfStatementDeathTest, Assert_DifferentGenerationID_Cond) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b2.Expr(true), b1.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementDeathTest, Assert_DifferentGenerationID_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b1.Expr(true), b2.Block());
        },
        "internal compiler error");
}

TEST_F(IfStatementDeathTest, Assert_DifferentGenerationID_ElseStatement) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.If(b1.Expr(true), b1.Block(), b2.Else(b2.If(b2.Expr("ident"), b2.Block())));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
