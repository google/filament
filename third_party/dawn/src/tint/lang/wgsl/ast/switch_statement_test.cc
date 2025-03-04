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

#include "src/tint/lang/wgsl/ast/switch_statement.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using SwitchStatementTest = TestHelper;
using SwitchStatementDeathTest = SwitchStatementTest;

TEST_F(SwitchStatementTest, Creation) {
    auto* case_stmt = create<CaseStatement>(tint::Vector{CaseSelector(1_u)}, Block());
    auto* ident = Expr("ident");
    tint::Vector body{case_stmt};

    auto* stmt = create<SwitchStatement>(ident, body, tint::Empty, tint::Empty);
    EXPECT_EQ(stmt->condition, ident);
    ASSERT_EQ(stmt->body.Length(), 1u);
    EXPECT_EQ(stmt->body[0], case_stmt);
}

TEST_F(SwitchStatementTest, Creation_WithSource) {
    auto* ident = Expr("ident");
    auto* stmt = create<SwitchStatement>(Source{Source::Location{20, 2}}, ident, tint::Empty,
                                         tint::Empty, tint::Empty);
    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(SwitchStatementTest, Creation_WithAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* ident = Expr("ident");
    auto* stmt =
        create<SwitchStatement>(ident, tint::Empty, tint::Vector{attr1, attr2}, tint::Empty);

    EXPECT_THAT(stmt->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(SwitchStatementTest, Creation_WithBodyAttributes) {
    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* ident = Expr("ident");
    auto* stmt =
        create<SwitchStatement>(ident, tint::Empty, tint::Empty, tint::Vector{attr1, attr2});

    EXPECT_THAT(stmt->body_attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(SwitchStatementTest, IsSwitch) {
    tint::Vector lit{CaseSelector(2_i)};
    auto* ident = Expr("ident");
    tint::Vector body{create<CaseStatement>(lit, Block())};

    auto* stmt = create<SwitchStatement>(ident, body, tint::Empty, tint::Empty);
    EXPECT_TRUE(stmt->Is<SwitchStatement>());
}

TEST_F(SwitchStatementDeathTest, Assert_Null_Condition) {
    using CaseStatementList = tint::Vector<const CaseStatement*, 2>;
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            CaseStatementList cases;
            cases.Push(
                b.create<CaseStatement>(tint::Vector{b.CaseSelector(b.Expr(1_i))}, b.Block()));
            b.create<SwitchStatement>(nullptr, cases, tint::Empty, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementDeathTest, Assert_Null_CaseStatement) {
    using CaseStatementList = tint::Vector<const CaseStatement*, 2>;
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<SwitchStatement>(b.Expr(true), CaseStatementList{nullptr}, tint::Empty,
                                      tint::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementDeathTest, Assert_DifferentGenerationID_Condition) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<SwitchStatement>(b2.Expr(true),
                                       tint::Vector{
                                           b1.create<CaseStatement>(
                                               tint::Vector{
                                                   b1.CaseSelector(b1.Expr(1_i)),
                                               },
                                               b1.Block()),
                                       },
                                       tint::Empty, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(SwitchStatementDeathTest, Assert_DifferentGenerationID_CaseStatement) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<SwitchStatement>(b1.Expr(true),
                                       tint::Vector{
                                           b2.create<CaseStatement>(
                                               tint::Vector{
                                                   b2.CaseSelector(b2.Expr(1_i)),
                                               },
                                               b2.Block()),
                                       },
                                       tint::Empty, tint::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
