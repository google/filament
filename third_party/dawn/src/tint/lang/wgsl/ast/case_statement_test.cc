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

#include "src/tint/lang/wgsl/ast/case_statement.h"

#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using CaseStatementTest = TestHelper;
using CaseStatementDeathTest = CaseStatementTest;

TEST_F(CaseStatementTest, Creation_i32) {
    auto* selector = CaseSelector(2_i);
    tint::Vector b{selector};

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(tint::Vector{discard}, tint::Empty);

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.Length(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.Length(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, Creation_u32) {
    auto* selector = CaseSelector(2_u);
    tint::Vector b{selector};

    auto* discard = create<DiscardStatement>();
    auto* body = create<BlockStatement>(tint::Vector{discard}, tint::Empty);

    auto* c = create<CaseStatement>(b, body);
    ASSERT_EQ(c->selectors.Length(), 1u);
    EXPECT_EQ(c->selectors[0], selector);
    ASSERT_EQ(c->body->statements.Length(), 1u);
    EXPECT_EQ(c->body->statements[0], discard);
}

TEST_F(CaseStatementTest, ContainsDefault_WithDefault) {
    tint::Vector b{CaseSelector(2_u), DefaultCaseSelector()};
    auto* c = create<CaseStatement>(b, create<BlockStatement>(tint::Empty, tint::Empty));
    EXPECT_TRUE(c->ContainsDefault());
}

TEST_F(CaseStatementTest, ContainsDefault_WithOutDefault) {
    tint::Vector b{CaseSelector(2_u), CaseSelector(3_u)};
    auto* c = create<CaseStatement>(b, create<BlockStatement>(tint::Empty, tint::Empty));
    EXPECT_FALSE(c->ContainsDefault());
}

TEST_F(CaseStatementTest, Creation_WithSource) {
    tint::Vector b{CaseSelector(2_i)};

    auto* body = create<BlockStatement>(
        tint::Vector{
            create<DiscardStatement>(),
        },
        tint::Empty);
    auto* c = create<CaseStatement>(Source{Source::Location{20, 2}}, b, body);
    auto src = c->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CaseStatementTest, IsCase) {
    auto* c = create<CaseStatement>(tint::Vector{DefaultCaseSelector()},
                                    create<BlockStatement>(tint::Empty, tint::Empty));
    EXPECT_TRUE(c->Is<CaseStatement>());
}

TEST_F(CaseStatementDeathTest, Assert_Null_Body) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(tint::Vector{b.DefaultCaseSelector()}, nullptr);
        },
        "internal compiler error");
}

TEST_F(CaseStatementDeathTest, Assert_Null_Selector) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<CaseStatement>(tint::Vector<const ast::CaseSelector*, 1>{nullptr},
                                    b.create<BlockStatement>(tint::Empty, tint::Empty));
        },
        "internal compiler error");
}

TEST_F(CaseStatementDeathTest, Assert_DifferentGenerationID_Call) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(tint::Vector{b1.DefaultCaseSelector()},
                                     b2.create<BlockStatement>(tint::Empty, tint::Empty));
        },
        "internal compiler error");
}

TEST_F(CaseStatementDeathTest, Assert_DifferentGenerationID_Selector) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<CaseStatement>(tint::Vector{b2.CaseSelector(b2.Expr(2_i))},
                                     b1.create<BlockStatement>(tint::Empty, tint::Empty));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
