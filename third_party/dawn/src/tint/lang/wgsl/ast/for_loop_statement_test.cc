// Copyright 2021 The Dawn & Tint Authors
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

using ForLoopStatementTest = TestHelper;
using ForLoopStatementDeathTest = ForLoopStatementTest;

TEST_F(ForLoopStatementTest, Creation) {
    auto* init = Decl(Var("i", ty.u32()));
    auto* cond = create<BinaryExpression>(core::BinaryOp::kLessThan, Expr("i"), Expr(5_u));
    auto* cont = Assign("i", Add("i", 1_u));
    auto* body = Block(Return());
    auto* l = For(init, cond, cont, body);

    EXPECT_EQ(l->initializer, init);
    EXPECT_EQ(l->condition, cond);
    EXPECT_EQ(l->continuing, cont);
    EXPECT_EQ(l->body, body);
}

TEST_F(ForLoopStatementTest, Creation_WithSource) {
    auto* body = Block(Return());
    auto* l = For(Source{{20u, 2u}}, nullptr, nullptr, nullptr, body);
    auto src = l->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ForLoopStatementTest, Creation_Null_InitCondCont) {
    auto* body = Block(Return());
    auto* l = For(nullptr, nullptr, nullptr, body);
    EXPECT_EQ(l->body, body);
}

TEST_F(ForLoopStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* body = Block(Return());
    auto* l = For(nullptr, nullptr, nullptr, body, tint::Vector{attr1, attr2});

    EXPECT_THAT(l->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(ForLoopStatementDeathTest, Assert_Null_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.For(nullptr, nullptr, nullptr, nullptr);
        },
        "internal compiler error");
}

TEST_F(ForLoopStatementDeathTest, Assert_DifferentGenerationID_Initializer) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.For(b2.Block(), nullptr, nullptr, b1.Block());
        },
        "internal compiler error");
}

TEST_F(ForLoopStatementDeathTest, Assert_DifferentGenerationID_Condition) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.For(nullptr, b2.Expr(true), nullptr, b1.Block());
        },
        "internal compiler error");
}

TEST_F(ForLoopStatementDeathTest, Assert_DifferentGenerationID_Continuing) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.For(nullptr, nullptr, b2.Block(), b1.Block());
        },
        "internal compiler error");
}

TEST_F(ForLoopStatementDeathTest, Assert_DifferentGenerationID_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.For(nullptr, nullptr, nullptr, b2.Block());
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
