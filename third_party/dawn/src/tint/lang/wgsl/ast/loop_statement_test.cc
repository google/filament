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

#include "src/tint/lang/wgsl/ast/loop_statement.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"

namespace tint::ast {
namespace {

using LoopStatementTest = TestHelper;
using LoopStatementDeathTest = LoopStatementTest;

TEST_F(LoopStatementTest, Creation) {
    auto* body = Block(create<DiscardStatement>());
    auto* b = body->Last();

    auto* continuing = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, continuing, tint::Empty);
    ASSERT_EQ(l->body->statements.Length(), 1u);
    EXPECT_EQ(l->body->statements[0], b);
    ASSERT_EQ(l->continuing->statements.Length(), 1u);
    EXPECT_EQ(l->continuing->statements[0], continuing->Last());
}

TEST_F(LoopStatementTest, Creation_WithSource) {
    auto* body = Block(create<DiscardStatement>());

    auto* continuing = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(Source{Source::Location{20, 2}}, body, continuing, tint::Empty);
    auto src = l->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(LoopStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");

    auto* body = Block(Return());
    auto* l = create<LoopStatement>(body, nullptr, tint::Vector{attr1, attr2});

    EXPECT_THAT(l->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(LoopStatementTest, IsLoop) {
    auto* l = create<LoopStatement>(Block(), Block(), tint::Empty);
    EXPECT_TRUE(l->Is<LoopStatement>());
}

TEST_F(LoopStatementTest, HasContinuing_WithoutContinuing) {
    auto* body = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, nullptr, tint::Empty);
    EXPECT_FALSE(l->continuing);
}

TEST_F(LoopStatementTest, HasContinuing_WithContinuing) {
    auto* body = Block(create<DiscardStatement>());

    auto* continuing = Block(create<DiscardStatement>());

    auto* l = create<LoopStatement>(body, continuing, tint::Empty);
    EXPECT_TRUE(l->continuing);
}

TEST_F(LoopStatementDeathTest, Assert_Null_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<LoopStatement>(nullptr, nullptr, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(LoopStatementDeathTest, Assert_DifferentGenerationID_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<LoopStatement>(b2.Block(), b1.Block(), tint::Empty);
        },
        "internal compiler error");
}

TEST_F(LoopStatementDeathTest, Assert_DifferentGenerationID_Continuing) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<LoopStatement>(b1.Block(), b2.Block(), tint::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
